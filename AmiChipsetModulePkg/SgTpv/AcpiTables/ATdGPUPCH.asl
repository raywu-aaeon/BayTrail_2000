//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/ATdGPUPCH.asl 1     1/15/13 6:02a Joshchou $
//
// $Revision: 1 $
//
// $Date: 1/15/13 6:02a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/ATdGPUPCH.asl $
// 
//**********************************************************************
Scope(ASL_DGPUPCH_SCOPE)
{   
//    OperationRegion (PEGR, PCI_Config, 0, 0x100)
//    Field(PEGR, DWordAcc, Lock, Preserve)
//    {
//        Offset(0x4C),
//        SSID, 32,
//    }        

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    _ON
//
// Description:  dGPU power ON control method
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>    
    Method(_ON,0,Serialized)
    {
        HGON()  // OEM Mxm Power On

        //Set the SSID for the ATI MXM
//        Store(MXM_SSVID_DID, SSID)

        //Ask OS to do a PnP rescan
        Notify(ASL_SG_ULT_RP_NUM,0)                      
    }

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    _OFF
//
// Description:  dGPU power OFF control method
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>    
    Method(_OFF,0,Serialized)
    {
        HGOF()  // OEM Mxm Power On

        //Ask OS to do a PnP rescan
        Notify(ASL_SG_ULT_RP_NUM,0)
    }

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    _STA
//
// Description:  Returns curent dGPU power/presence state
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>    
    Method(_STA,0,Serialized)
    {
        Return(SGST())  // OEM Mxm Power status 
    }

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    _INI
//
// Description:  dGPU Init control method. Used to force dGPU _ADR to return proper PCI address
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>    
//    Method (_INI)
//    {       
// should already be set by now...
////        Store(MXM_SSVID_DID, SSID) //Set the SSID for the ATI MXM
//        Store(0x0, ASL_DGPUPCH_SCOPE._ADR) //make sure PEGP address returns 0x00000000
//    }

} // end Scope(ASL_DGPUPCH_SCOPE)
