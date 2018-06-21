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
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Library/AmiPciBusLib.h>

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------


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
EFI_STATUS EFIAPI AmiPciExpressLibConstructor(IN EFI_HANDLE  ImageHandle, IN EFI_SYSTEM_TABLE  *SystemTable)
{
	EFI_STATUS				Status=EFI_SUCCESS;
//-------------------------------------------------
	InitAmiLib(ImageHandle, SystemTable);

	return Status;;
}
*/


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
EFI_STATUS Pcie3EqualizeLink(PCI_DEV_INFO *Device, BOOLEAN *LnkRetrain){
    EFI_STATUS          Status=EFI_SUCCESS;
	PCI_CFG_ADDR		addr;
    PCIE_LNK_CNT3_REG   LnkCnt3;
    PCIE_LNK_STA2_REG   LnkSta2;
//-----------------------------

    //If device don't have GEN 3 Cap Hdr, or link is operating on lell then 8.0 GT/s speed - just exit...
    if(Device->PciExpress->Pcie3==NULL) return EFI_SUCCESS;

    //Get LNK_STA2 regiser
	addr.ADDR=Device->Address.ADDR;
    addr.Addr.Register=Device->PciExpress->PcieOffs+PCIE_LNK_STA2_OFFSET;
    Status=PciCfg16(Device->RbIo, addr, FALSE,&LnkSta2.LNK_STA2);
	ASSERT_EFI_ERROR(Status);

    //Display Content of the LNK_STA2 register.
    PCI_TRACE((TRACE_PCI," LNK_STA2-> [R=%X|EqReq=%X|EqP3=%X|EqP2=%X|EqP1=%X|EqCompl=%X|SelDeEmp=%X]\n",
        LnkSta2.Reserved,
        LnkSta2.EqRequest,
        LnkSta2.EqPhase3Ok,
        LnkSta2.EqPhase2Ok,
        LnkSta2.EqPhase1Ok,
        LnkSta2.EqComplete,
        LnkSta2.SelDeEmphasis));

    //Check if equalization was requested or we are about to enter lLNK training session...
    if(LnkSta2.EqRequest || *LnkRetrain){

        PCI_TRACE((TRACE_PCI," PciE3: Equalization for Device @ [B%X|D%X|F%X] LnkRetrain=%X Before\n",
            addr.Addr.Bus,addr.Addr.Device,addr.Addr.Function, *LnkRetrain));
        //read Lnk Control 3 register in Sec PCIe Ext Cap Header.
        addr.Addr.ExtendedRegister=Device->PciExpress->Pcie3->SecPcieCapOffs+PCIE_LNK_CNT3_OFFSET;
        Status=PciCfg32(Device->RbIo, addr, FALSE,&LnkCnt3.LNK_CNT3);
	    ASSERT_EFI_ERROR(Status);

        //Set Prform Equalization bit and disable Equalization Request Interrupt, just in case.
        LnkCnt3.LnkEqReqIntEn=0;
        LnkCnt3.PerformEqualiz=1;

        //Write it back into LNK_CNT3 register.
        Status=PciCfg32(Device->RbIo, addr, TRUE, &LnkCnt3.LNK_CNT3);
	    ASSERT_EFI_ERROR(Status);

        *LnkRetrain=TRUE;

        //Display Content of the LNK_CNT3 register.
        PCI_TRACE((TRACE_PCI," LNK_CNT3-> [R=%X|LnkEqReqIntEn=%X|DoEq=%X] LnkRetrain=%X After\n",
            LnkCnt3.Reserved,
            LnkCnt3.LnkEqReqIntEn,
            LnkCnt3.PerformEqualiz,
            *LnkRetrain));

    }
    return Status;
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
EFI_STATUS Pcie3GetEqualizationStatus(PCI_DEV_INFO *Device){
	PCI_CFG_ADDR		addr;
    PCIE_LNK_STA2_REG   LnkSta2;
    EFI_STATUS          Status=EFI_SUCCESS;
//-----------------------------

    //If device don't have GEN 3 Cap Hdr, or link is operating on lell then 8.0 GT/s speed - just exit...
    if(Device->PciExpress->Pcie3==NULL) return EFI_SUCCESS;

    //Get LNK_STA2 regiser
	addr.ADDR=Device->Address.ADDR;
    addr.Addr.Register=Device->PciExpress->PcieOffs+PCIE_LNK_STA2_OFFSET;
    Status=PciCfg16(Device->RbIo, addr, FALSE,&LnkSta2.LNK_STA2);
	ASSERT_EFI_ERROR(Status);

    //Display Content of the LNK_STA2 register.
    PCI_TRACE((TRACE_PCI,"Pcie3-> Checking Equalization Status...\n LNK_STA2-> [R=%X|EqReq=%X|EqP3=%X|EqP2=%X|EqP1=%X|EqCompl=%X|SelDeEmp=%X]\n",
        LnkSta2.Reserved,
        LnkSta2.EqRequest,
        LnkSta2.EqPhase3Ok,
        LnkSta2.EqPhase2Ok,
        LnkSta2.EqPhase1Ok,
        LnkSta2.EqComplete,
        LnkSta2.SelDeEmphasis));

    //Check equalization sesults...
    if (LnkSta2.EqComplete) return EFI_SUCCESS;

    //Check if any of the equalization phases were completed...
    if(LnkSta2.LNK_STA2 & 0x1C) return EFI_NOT_AVAILABLE_YET;

    return EFI_DEVICE_ERROR;
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
VOID Pcie3InitDisplayPcie3Data(PCI_DEV_INFO *Device){
    //Do some additional checks if device has GEN 3 Secondary PCIe Cap Header.
    //At that point if we have discovered Secondary PCIe Cap HDR Device->PciExpress->Pcie3 must be initialized.
    if(Device->PciExpress->Pcie3 != NULL){


        //Update Speed encoding it is defined differently for devices Supporting V3.0 spec.

        Device->PciExpress->Pcie3->MaxLanesCount=Device->PciExpress->LinkCap.MaxLnkWidth;

        //Display content of LNK_CAP2_REG
        PCI_TRACE((TRACE_PCI,"PCIe3 -> Device is PCIe v3.0 Compliant!!! LNK_CAP2 present!!! \n LNK_CAP2-> [R2=%X|CrossL=%X|SuppLnkSpeedVect=%X|R1=%X]; LANE_ERR_STA->[%X]; MaxLanes=%X\n",
            Device->PciExpress->Pcie2->LinkCap2.Reserved2,
            Device->PciExpress->Pcie2->LinkCap2.CrossLnk,
            Device->PciExpress->Pcie2->LinkCap2.SuppLnkSpeeds,
            Device->PciExpress->Pcie2->LinkCap2.Reserved1,
            Device->PciExpress->Pcie3->LaneErrSts,
            Device->PciExpress->Pcie3->MaxLanesCount));

    }
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
EFI_STATUS Pcie3AllocateInitPcie3Data(PCI_DEV_INFO *Device, UINT32 Pcie3CapOffset){
	PCI_CFG_ADDR	addr;
//---------------------------
	PCI_TRACE((TRACE_PCI,"PciE3: Found SEC PCIe Ext Cap Header @ offset 0x%X\n", Pcie3CapOffset));

	//Allocate Memory
	Device->PciExpress->Pcie3=MallocZ(sizeof(PCIE3_DATA));
	if(Device->PciExpress->Pcie3==NULL) {
		ASSERT_EFI_ERROR(EFI_OUT_OF_RESOURCES);
		return EFI_OUT_OF_RESOURCES;
	}

	Device->PciExpress->Pcie3->SecPcieCapOffs=Pcie3CapOffset;

	//Get Content of LaneErrSts and MaxLanesCount
    addr.ADDR=Device->Address.ADDR;
    addr.Addr.ExtendedRegister=Pcie3CapOffset+PCIE_LANE_ERR_STA_OFFSET;

	return PciCfg32(Device->RbIo, addr, FALSE,&Device->PciExpress->Pcie3->LaneErrSts);
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
BOOLEAN Pcie3CheckPcie3Compatible(PCI_DEV_INFO	*Device){
	
	if(	Device->PciExpress!=NULL &&
		Device->PciExpress->Pcie2!=NULL &&
		Device->PciExpress->Pcie3!=NULL &&
		Device->DevSetup.Pcie1Disable == 0 &&
		Device->DevSetup.Pcie2Disable == 0
	)return TRUE;
	else return FALSE;
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

