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
#ifndef _AMI_PCI_HOT_PLUG_LIB_H_
#define _AMI_PCI_HOT_PLUG_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------

#include <Pci.h>  
#include <PciSetup.h>
#include <PciBus.h>
#include <PciHostBridge.h>
#include <AmiDxeLib.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciHotplugInit.h> //MdeModulePkg Spelling

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
//extern EFI_PCI_HOT_PLUG_INIT_PROTOCOL	*gRhpcInitProtocol;

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
EFI_STATUS HpCheckRootHotplug(PCI_DEV_INFO *Device, UINT8 MaxBusFound);


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
BOOLEAN HpIsHpb(PCI_DEV_INFO *Device);

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
EFI_STATUS HpGetHpbResPadding(PCI_DEV_INFO *Device);


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
EFI_STATUS HpAllocateInitHpData(PCI_DEV_INFO *Device, UINT16 HpCapOffset);

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
BOOLEAN HpCheckHpCompatible(PCI_DEV_INFO *Device);

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
EFI_STATUS HpLocateProtocolSortRhpc(PCI_HOST_INFO *PciHpcHost, PCI_DEV_INFO	*RootBridge);


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

EFI_STATUS HpApplyBusPadding(PCI_DEV_INFO *Brg, UINT8 OldMaxBus, UINT8 *MaxBusFound);


// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: LibConstructor
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
VOID HpClearPaddingData(PCI_BAR *PaddingBar);

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
EFI_STATUS HpApplyResPadding(PCI_BAR *PaddingBar, PCI_BAR *BridgeBar);


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
EFI_STATUS HpcInstallHpProtocol (PCI_HOST_BRG_DATA *HostBrg	);

EFI_STATUS HpcFindSlots(PCI_HOST_BRG_DATA *HostBrg, PCI_ROOT_BRG_DATA *RootBrg);



EFI_STATUS HpcGetResourcePadding(IN  EFI_PCI_HOT_PLUG_INIT_PROTOCOL *This,
                                 IN  EFI_DEVICE_PATH_PROTOCOL       *HpcDevicePath,
                                 IN  UINT64                         HpcPciAddress,
                                 OUT EFI_HPC_STATE                  *HpcState,
                                 OUT VOID                           **Padding,
                                 OUT EFI_HPC_PADDING_ATTRIBUTES     *Attributes);




VOID HpcFillDescriptor(ASLR_QWORD_ASD *Descriptor, HP_PADD_RES_TYPE PaddType, UINT64  Length);


EFI_STATUS HpcInitInitializeRootHpc(IN EFI_PCI_HOT_PLUG_INIT_PROTOCOL       *This,
                                    IN  EFI_DEVICE_PATH_PROTOCOL            *HpcDevicePath,
                                    IN  UINT64                              HpcPciAddress,
                                    IN  EFI_EVENT                         	Event, OPTIONAL
                                    OUT EFI_HPC_STATE                    	*HpcState);




BOOLEAN HpcCheckHpCompatibleHost(PCI_HOST_BRG_DATA *HostBrg);



/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif //_AMI_PCI_BUS_COMMON_LIB_H_


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

