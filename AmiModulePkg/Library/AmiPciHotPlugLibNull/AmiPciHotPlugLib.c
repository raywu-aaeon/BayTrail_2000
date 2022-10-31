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
// Name:  AmiSioDxeLib.C
//
// Description: Library Class for AMI SIO Driver.
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
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciHotplugInit.h>
//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
//ONLY ONE INSTANCE ALOUD IN THE SYSTEM!!!
EFI_PCI_HOT_PLUG_INIT_PROTOCOL	*gRhpcInitProtocol=NULL;
BOOLEAN 						gRhpcNotFound=FALSE;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HpcCheckHpCompatibleHost()
//
// Description: This function will check if "HostBrg" passed is the Bridge
// with hotplug support.
//
// Input:
//  PCI_HOST_BRG_DATA *HostBrg     Pointer to PCI HB Private Data structure.
//
// Output:  BOOLEAN
//  TRUE    If "Device" is a Bridge with HPC on it.
//  FALSE   Otherwice.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN HpcCheckHpCompatibleHost(PCI_HOST_BRG_DATA *HostBrg){
	return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	IsHpb()
//
// Description: This function will check if "Device" passed is the Bridge
// Type Device with hotplug support.
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//
// Output:  BOOLEAN
//  TRUE    If "Device" is a Bridge with HPC on it.
//  FALSE   Otherwice.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN HpIsHpb(PCI_DEV_INFO *Device){

	return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	CheckRootHotplug()
//
// Description: This function will update pointer to PCI_RHPC_INFO of
// Bridge Type device which creates a hot plug bus. Also if "Device"
// creates the 'home bus' for Root HPC it will initialize Root HPC and
// record the HPC state;
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_NOT_FOUND           When Device not present in the system.
//  EFI_OUT_OF_RESOURCES    When system runs out of resources.
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpCheckRootHotplug(PCI_DEV_INFO *Device, UINT8 MaxBusFound){
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetHpbResPadding()
//
// Description: This function will get and apply resource padding
// requirements for the PCI to PCI Bridge or Card Bus Bridge, if this
// Bridge is supports hot plug.
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_NOT_FOUND           When Device not present in the system.
//  EFI_OUT_OF_RESOURCES    When system runs out of resources.
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpGetHpbResPadding(PCI_DEV_INFO *Device){
	return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
BOOLEAN HpCheckHpCompatible(PCI_DEV_INFO *Device){
	return FALSE;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpAllocateInitHpData(PCI_DEV_INFO *Device, UINT16 HpCapOffset){
	return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpLocateProtocolSortRhpc(PCI_HOST_INFO	*PciHpcHost, PCI_DEV_INFO	*RootBridge){
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
VOID HpClearPaddingData(PCI_BAR *PaddingBar){
	return;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpApplyResPadding(PCI_BAR *PaddingBar, PCI_BAR *BridgeBar){
	return EFI_UNSUPPORTED;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpApplyBusPadding(PCI_DEV_INFO *Brg, UINT8 OldMaxBus, UINT8 *MaxBusFound){
	return EFI_SUCCESS;

}

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpcUpdateLocationData(
								PCI_HOST_BRG_DATA 		*HostBrg, 
								PCI_ROOT_BRG_DATA 		*RootBrg, 
								AMI_SDL_PCI_DEV_INFO	*HpBrgSdlData,
								UINTN					HpBrgIndex)
{
	return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HpcFindHpSlots
//
// Description:
//  This function find
//  information and initialize an instance of PCI Root Hotplug Controller
//  Initialization Protocol.
//
// Input:		PCI_HOST_BRG_DATA   *HostBrg - Pointer on Private Data
//              structure for which PCI Hot Plug Init Protocol going to
//              be initialized
//
// Output:		EFI_SUCCESS is OK
//              EFI_NOT_FOUND no Hotplug slots where found.
//
// Notes:		CHIPSET AND/OR BOARD PORTING NEEDED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpcFindSlots(PCI_HOST_BRG_DATA *HostBrg, PCI_ROOT_BRG_DATA *RootBrg)
{
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HpcInitGetRootHpcList
//
// Description:	GetRootHpcList() Function of the PCI Root Hotplug Controller
// Initialization Protocol. See PI 1.1 Spec or details
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpcInitGetRootHpcList(IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL      *This,
                                 OUT UINTN                              *HpcCount,
                                 OUT EFI_HPC_LOCATION                   **HpcList)
{
    return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS FindHpcLocData(  IN EFI_DEVICE_PATH_PROTOCOL *HpcDevicePath,
                            IN PCI_HPC_DATA             *HpcData,
                            OUT HPC_LOCATION_DATA       **HpcLocData )
{
    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HpcInitInitializeRootHpc
//
// Description:	InitializeRootHpc() Function of the PCI Root Hotplug Controller
// Initialization Protocol. See PI 1.1 Spec or details
//
// NOTE:
//  This function call will be used to overwrite default resource Padding settings.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpcInitInitializeRootHpc(IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL       *This,
                                    IN  EFI_DEVICE_PATH_PROTOCOL            *HpcDevicePath,
                                    IN  UINT64                              HpcPciAddress,
                                    IN  EFI_EVENT                         	Event, OPTIONAL
                                    OUT EFI_HPC_STATE                    	*HpcState)
{
    return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
VOID HpcFillDescriptor(ASLR_QWORD_ASD *Descriptor, HP_PADD_RES_TYPE PaddType, UINT64  Length){
    return;
}


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure:
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
EFI_STATUS HpcGetResourcePadding(IN  EFI_PCI_HOT_PLUG_INIT_PROTOCOL *This,
                                 IN  EFI_DEVICE_PATH_PROTOCOL       *HpcDevicePath,
                                 IN  UINT64                         HpcPciAddress,
                                 OUT EFI_HPC_STATE                  *HpcState,
                                 OUT VOID                           **Padding,
                                 OUT EFI_HPC_PADDING_ATTRIBUTES     *Attributes)
{
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	HpcInitProtocol
//
// Description:	This function will collect all information HP related
// information and initialize an instance of PCI Root Hotplug Controller
// Initialization Protocol.
//
// Input:		PCI_HOST_BRG_DATA   *HostBrg - Pointer on Private Data
//              structure for which PCI Hot Plug Init Protocol going to
//              be initialized
//
// Output:		EFI_SUCCESS is OK
//              EFI_NOT_FOUND no Hotplug slots where found.
//
// Notes:		CHIPSET AND/OR BOARD PORTING NEEDED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HpcInstallHpProtocol(PCI_HOST_BRG_DATA *HostBrg)
{
    return EFI_SUCCESS;
}

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

