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
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProbeSriovDevice()
//
// Description: This function will collect information about SRIOV PCIE Device
// and initialize it based on information collected.
//
// Input:
//  PCI_DEV_INFO    *Device Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_OUT_OF_RESOURCES    When system runs out of resources.
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SriovProbeDevice(PCI_DEV_INFO *Device, UINT32 SriovCapBaseOffset, UINT8 *MaxBusFound){
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
EFI_STATUS SriovAllocateInitSriovData(PCI_DEV_INFO *Device, UINT32 SriovCapOffset, UINT8 *MaxBusFound ){
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
BOOLEAN SriovCheckSriovCompatible(PCI_DEV_INFO	*Device){
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
EFI_STATUS	SriovDbAddVirtualBar(PCI_DEV_INFO *Device, DBE_DATABASE *Db, MRES_TYPE ResType){
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
BOOLEAN SriovCheckBarType(PCI_DEV_INFO *Device, PCI_BAR_TYPE BarType){
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
VOID SriovConvertResources(	PCI_DEV_INFO *Device, PCI_BAR_TYPE NarrowType, PCI_BAR_TYPE WideType,
							RES_CONV_TYPE ConvType, BOOLEAN NeedToConvert, BOOLEAN CombineMemPmem)
{
	return;
}


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

