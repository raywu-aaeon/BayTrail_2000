//****************************************************************************
//****************************************************************************
//**                                                                        **
//**         (C)Copyright 1985-2013, American Megatrends, Inc.              **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
//****************************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/NViGDmiscPCH.asl 1     1/15/13 6:03a Joshchou $
//
// $Revision: 1 $
//
// $Date: 1/15/13 6:03a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/NViGDmiscPCH.asl $
// 
// 
// 
//**********************************************************************

External(DSEN)
External(ASL_IGPU_SCOPE.CDCK)
External(ASL_DGPUPCH_SCOPE.DD02)
External(ASL_IGPU_SCOPE.IDAB, MethodObj)

//Do not remove this function.
Method(IDAB, 0, Serialized)
{
    If (CondRefOf(ASL_IGPU_SCOPE.IDAB))
    {
        ASL_IGPU_SCOPE.IDAB()
    }
}

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    HGAS
//
// Description:  HG Adaptor select, notify
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method(HGAS) //HG Adaptor select
{
    //Stateless button/Hotkey supporting 3 states - Power Saver, Adaptive and Perf
    
    Increment(ASL_IGPU_SCOPE.GPSS)
    Mod(ASL_IGPU_SCOPE.GPSS, 3, ASL_IGPU_SCOPE.GPSS)

    Store(1,ASL_IGPU_SCOPE.GPPO)
    Store(1,ASL_IGPU_SCOPE.SGNC)    //indicate 'policy select' change

    Notify(ASL_IGPU_SCOPE, 0xD0)
    Notify(ASL_PCI_SCOPE.WMI1, 0xD0)    // Mirror Notify on WMI1

}

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    HBRT
//
// Description:  Send backlight notifications to the DGPU LFP device
//              This is required for Win7 and is backward compatible with Vista
//
// Input:       Arg0 - 4 - Brightnes Down, 3- Up
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method(HBRT, 1 , Serialized)
{
    Store(Arg0,ASL_IGPU_SCOPE.DACE)
  
    If(And(4,DSEN))     //Note: DSEN variable is expected to be set by IGD miniport only.
    {
        If(LEqual(Arg0,4))
        {   
            Notify(ASL_DGPUPCH_SCOPE.DD02,0x87)  //Note: DD02 is hardcoded as the LFP device in intelgfx.asl
        }
        If(LEqual(Arg0,3))
        {
            Notify(ASL_DGPUPCH_SCOPE.DD02,0x86)
        }
    }       
    
}

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    HPFS
//
// Description:  Panel Fitting Hot Key
//
// Input:       None
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method(HPFS, 0, Serialized)     //Panel Fitting Hot Key
{
    //
    // HG Handling of Panel Fitting Switch
    //

    Store(5,ASL_IGPU_SCOPE.DACE)  // Indicate display scaling hot key event
    Store(2,ASL_IGPU_SCOPE.SGNC)  // Indicate platpolicy change

    //
    // Expansion Mode toggling
    //
    Increment(ASL_IGPU_SCOPE.GPSP)
    Mod(ASL_IGPU_SCOPE.GPSP, 4 , ASL_IGPU_SCOPE.GPSP)    

    Notify(ASL_IGPU_SCOPE,0xDC)

}


//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    HNOT
//
// Description:  Notification handler for Switchable graphics. Called from GNOT()
//
// Input:	Arg0 = Current event type:
//			1 = display switch
//			2 = lid
//			3 = dock (!!!!!Acpi ref code, Method(GDCK)sends 4 for Dock notifications )
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method (HNOT, 1, Serialized)
{
    Switch(ToInteger(Arg0))
    {
        case(1) //Display Switch event
        {            
            Store(3,ASL_IGPU_SCOPE.SGNC)    // indicate 'Displaystatus' change
            Store(1,ASL_IGPU_SCOPE.DACE)
            Notify(ASL_IGPU_SCOPE, 0x80)
            Notify(ASL_PCI_SCOPE.WMI1, 0x80)    // Mirror Notify on WMI1
        }

        case (2) //Lid switch event
        {
            //Note: NV clarified that only LDES needs to be set           
            Store(1,ASL_IGPU_SCOPE.LDES)            
            Notify(ASL_IGPU_SCOPE, 0xDB)
            Notify(ASL_PCI_SCOPE.WMI1, 0xDB)    // Mirror Notify on WMI1
        }
//        case (3) //Dock event
        case (4) //Dock event (!!!Acpi ref code, Method(GDCK)sends 4 for Dock notifications)
        {
            Store(ASL_IGPU_SCOPE.CDCK, ASL_IGPU_SCOPE.DKST) // Store the current dock state
            Notify(ASL_IGPU_SCOPE, 0x81)     
            Notify(ASL_PCI_SCOPE.WMI1, 0x81)    // Mirror Notify on WMI1
        }

        Default 
        {
            Notify(ASL_IGPU_SCOPE,0x80)
        }
        
    }

}

//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    CHPS
//
// Description:  Shows current Hybrid Policy status on Port80 header
//         Adaptive -> 1, Save power -> 2 and High performance -> 3
//
// Input:	None
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method(CHPS)
{
    P8DB(0xEC, Add(ASL_IGPU_SCOPE.GPSS, 1), 2000)
}


//<AMI_PHDR_START>
//------------------------------------------------------------------------
//
// Procedure:    HDOS
//
// Description:  Check if the _DOS flag was set during the hot key handling
//
// Input:	None
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method(HDOS, 0, Serialized)
{
    If(LEqual(ASL_IGPU_SCOPE.DOSF,1))
    {                
        Store(1,ASL_IGPU_SCOPE.SGNC)    // indicate 'policy select' change
        Notify(ASL_IGPU_SCOPE,0xD0)
        Notify(ASL_PCI_SCOPE.WMI1, 0xD0)    // Mirror Notify on WMI1
        Store(0, ASL_IGPU_SCOPE.DOSF)   // Clear the DOSF
    }
}
