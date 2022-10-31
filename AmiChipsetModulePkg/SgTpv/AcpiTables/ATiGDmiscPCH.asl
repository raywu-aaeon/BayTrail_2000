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
// $Header: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/ATiGDmiscPCH.asl 1     1/15/13 6:02a Joshchou $
//
// $Revision: 1 $
//
// $Date: 1/15/13 6:02a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SharkBayRefCodes/SwitchableGraphics/Sg TPV/Sg Acpi Tables/SgTpvPCH/ATiGDmiscPCH.asl $
// 
//**********************************************************************

External(DSEN)
External(ASL_IGPU_SCOPE.AINT, MethodObj)
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
Method(HGAS)        // HG Adaptor Select
{
    // Toggle GFX Adapter.
    Store(1,ASL_IGPU_SCOPE.PXGS)
    Notify(ASL_IGPU_SCOPE,0x81)
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
    If(LNotEqual(ASL_IGPU_SCOPE.GSTP,1))
    {

        //
        // HG Handling of Panel Fitting Switch
        //
        If(LEqual(ASL_IGPU_SCOPE.AGXA,0))
        {
            ASL_IGPU_SCOPE.AINT(2, 0)
        }
        Else
        {
            Store(1,ASL_IGPU_SCOPE.EMDR)

            //
            // Expansion Mode toggling
            //
            If(LEqual(ASL_IGPU_SCOPE.EXPM,2))
            {
                Store(0,ASL_IGPU_SCOPE.EXPM)
            }
            Else
            {
                Increment(ASL_IGPU_SCOPE.EXPM)
            }
            Notify(ASL_IGPU_SCOPE,0x81)
        }
    }
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
//			3 = dock (!!!!!Calpella ref code, Method(GDCK)sends 4 for Dock notifications )
//-------------------------------------------------------------------------
//<AMI_PHDR_END> 
Method (HNOT, 1, Serialized)
{
    
    Switch(ToInteger(Arg0))
    {
        case(1) //Display Switch event
        {
            If(LNotEqual(ASL_IGPU_SCOPE.GSTP,1))
            {
                //
                // HG Handling of Display Switch Event
                //
                // Muxless?                              
                If(LEqual(SGMD,0x2))
                {
                     If(LEqual(ASL_IGPU_SCOPE.AGXA,0))
                     {
                      Notify(ASL_IGPU_SCOPE,0x80)
                     }

                     If(LEqual(ASL_IGPU_SCOPE.AGXA,1))
                     {
                      Notify(ASL_DGPUPCH_SCOPE,0x80)
                     }
                }
                Else
                {   
                    Store(1,ASL_IGPU_SCOPE.DSWR)
                    Notify(ASL_IGPU_SCOPE,0x81)
                }                
            }
        }
        case (2) //Lid switch event
        {
            // Muxless?                              
            If(LEqual(SGMD,0x2))
            {
                 If(LEqual(ASL_IGPU_SCOPE.AGXA,0))
                 {
                  Notify(ASL_IGPU_SCOPE,0x80)
                 }

                 If(LEqual(ASL_IGPU_SCOPE.AGXA,1))
                 {
                  Notify(ASL_DGPUPCH_SCOPE,0x80)
                 }
            }
            Else
            {
                Notify(ASL_IGPU_SCOPE,0x80)         //Placeholder for LID event
            }
        }
//        case (3) //Dock event
        case (4) //Dock event
        {
            // Muxless?                              
            If(LEqual(SGMD,0x2))
            {
                 If(LEqual(ASL_IGPU_SCOPE.AGXA,0))
                 {
                  Notify(ASL_IGPU_SCOPE,0x80)
                 }

                 If(LEqual(ASL_IGPU_SCOPE.AGXA,1))
                 {
                  Notify(ASL_DGPUPCH_SCOPE,0x80)
                 }
            }
            Else
            {
             Notify(ASL_IGPU_SCOPE,0x80)     //Placeholder for handling dock event
            }
        }
        Default 
        {
            Notify(ASL_IGPU_SCOPE,0x80)
        }
        
    }

}