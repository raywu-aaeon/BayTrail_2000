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
// $Header:$
//
// $Revision:  $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:    AmiPciExpressGen2Lib.h
//
// Description: 
//
// Notes:
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _AMI_PCI_EXPRESS_GEN_2_LIB_H_
#define _AMI_PCI_EXPRESS_GEN_2_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Efi.h>
#include <Pci.h>
#include <PciE.h>
#include <PciE21.h>
#include <PciE30.h>
#include <PciBus.h>
#include <AmiLib.h>
#include <AcpiRes.h>
#include <Protocol/PciIo.h>

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
//Variable, Prototype, and External Declarations
//-------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2GetGen2Info()
//
// Description: This function will collect information about PCIE GEN2 Device
// and initialize PCIE2_DATA structure based on information collected.
//
// Input:
//  PCI_DEV_INFO    *Device Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Pcie2GetGen2Info(PCI_DEV_INFO *Device);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2EnableAri()
//
// Description: This function will Enable ARI Forwarding in DownSream Port of
// the device passed if
//  1.Device referenced is an ARI device;
//  2.Parenting Bridge supports ARI Forwarding.
//  3.ARI Firvarding Setup Question Set to "Enabled"
//
// Input:
//  PCI_DEV_INFO    *Device Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_UNSUPPORTED         When Device or Parenting Bridge does not support ARI
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Pcie2EnableAri(PCI_DEV_INFO *Device, BOOLEAN EnableOption);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2CheckAri()
//
// Description: This function will Enable ARI Forwarding in DownSream Port of
// the device passed if
//  1.Device referenced is an ARI device;
//  2.Parenting Bridge supports ARI Forwarding.
//  3.ARI Firvarding Setup Question Set to "Enabled"
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//  BOOLEAN         *MultiFunc  Pointer to a Flag to modify if Device is MF Device
//  BOOLEAN         *AriEnabled Pointer to a Flag to modify if this function was able to ENABLE ARI.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Pcie2CheckAri(PCI_DEV_INFO *Device, BOOLEAN *MultiFunc, BOOLEAN *AriEnabled);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2SetLnkProperties()
//
// Description: This function will Select values for Link Control2 register on
// both sides of the LINK based on Setup Settings and hardware capabilities.
//
// Input:
//  PCI_DEV_INFO        *DnStreamPort   Pointer to PCI Device Private Data of Downstream Port of the link.
//  PCIE_LNK_CNT2_REG   *DnLnkCnt2      Pointer to the LNK_CNT2 Reg of the Downstream Port of the link.
//  PCI_DEV_INFO        *UpStreamPort   Pointer to PCI Device Private Data of Upwnstream Port of the link.
//  PCIE_LNK_CNT2_REG   *UpLnkCnt2      Pointer to the LNK_CNT2 Reg of the Downstream Port of the link.
//  BOOLEAN             *LinkHotResetRequired Flag to modify if Link will need HOT RESET after programming.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Pcie2SetLnkProperties(PCI_DEV_INFO           *DnStreamPort,
                                 PCIE_LNK_CNT2_REG      *DnLnkCnt2,
                                 PCI_DEV_INFO           *UpStreamPort,
                                 PCIE_LNK_CNT2_REG      *UpLnkCnt2,
                                 BOOLEAN                *LinkHotResetRequired,
                                 BOOLEAN                *LinkRetrainRequired
                                 );

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
BOOLEAN Pcie2CheckPcie2Compatible(PCI_DEV_INFO	*Device);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2SelectComplTimeOut()
//
// Description: This function will select appropriate Completion Timeout range
// from supported by the device.
//
// Input:
//  UINT32      Support     Supported by Device Completion Timeout ranges.
//  BOOLEAN     Short       A Flag to Indicate wahat type of ranges to select Biggest or Smallest
//
// Output:	UINT16
//  Value to be programmed in DEV_CNT2 Register.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 Pcie2SelectComplTimeOut(UINT32 Support, BOOLEAN Short);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   Pcie2SetDevProperties()
//
// Description: This function will Select values for DEVICE CCONTROL2 register
// based on Setup Settings and hardware capabilities.
//
// Input:
//  PCI_DEV_INFO        *DnStreamPort   Pointer to PCI Device Private Data of Downstream Port of the link.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Pcie2SetDevProperties(	PCI_DEV_INFO 		*Device);


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
EFI_STATUS Pcie2AllocateInitPcie2Data(PCI_DEV_INFO *Device);

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

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif //_AMI_PCI_EXPRESS_GEN_2_LIB_H_
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

