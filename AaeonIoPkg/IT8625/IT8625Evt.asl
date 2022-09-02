//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  <IT8625EVT.asl>
//
// Description: Define event handler for Super IO.
//
//<AMI_FHDR_END>
//**********************************************************************
//Scope(\_SB.PCI0.SBRG) {
//----------------------------------------------------------------------
// SET OF COMMON DATA/CONTROL METHODS USED FOR ALL LDN BASED SIO DEVICES
//----------------------------------------------------------------------
// SIO specific: SIOS - SIO Chipset specific code called from _PTS
// SIO specific: SIOW - SIO Chipset specific code called from _WAK
// SIO specific: SIOH - SIO event handler, to be called from correspondent _Lxx method
// SIO specific: PowerResources & _PR0 object to control Power management for FDC, LPT, UART1,2.

//----------------------------------------------------------------------
// SIO_PME WAKE UP EVENTS //
//----------------------------------------------------------------------
// Following code is the workaround for wake up on RI/Key/Mouse events
// are generated by some SuperIO. The wake up signal (SIO_PME) is
// connected to one of GPIOs of south bridge chip.
// Make sure the correspondent GPIO in south bridge is enabled to generate an SCI
//----------------------------------------------------------------------
// Add Flag for Fix WakeUp Switch bug. 
Name(PMFG, 0x00)    //PME wake status

//----------------------------------------------------------------------
// SIOS - SIO Chipset specific code called from _PTS
//----------------------------------------------------------------------
// input  - Arg0 : Sleep state #
// output - nothing
//----------------------------------------------------------------------
Method(SIOS, 1){
    // Aware wake up events in SIO chip
    Store("SIOS", Debug)

    //AMI_TODO: 
    //1. select sleep state
    If(LNotEqual(0x05, Arg0)){
        ^ENFG(0x04)                     //Set Logical Device 0A (PME)
        //2. clear PME Status
        Store(0xFF, ^OPT1)              //Clear I/O PME# Status
        //3. Enable PME /wakeup
        And(0xBF, ^OPT2, Local0)           //Enable PME# 
        Store(Local0, ^OPT2)
        //4. enable wake-up ; Enable Keyboard, PS/2 Mouse,    UART 1&2 to Generate PME.
        //ITE ApNote: 0xF0 Wakeup event enable must do at the end of whole wake up setting
        if(\KBFG){
            Or(^OPT0, 0x08, ^OPT0)      //enable Keyboard Wake-up bit
        }        
        Else{
            And(^OPT0, 0xF7, ^OPT0)     //disable Keyboard Wake-up bit
        }
        if(\MSFG){
            Or(^OPT0, 0x10, ^OPT0)      //enable Mouse Wake-up bit
        }        
        Else{
            And(^OPT0, 0xEF, ^OPT0)     //disable Mouse Wake-up bit
        }
        ^EXFG()
    }
}

//----------------------------------------------------------------------
// SIOW - SIO Chipset specific code called from _WAK
//----------------------------------------------------------------------
// input  - Sleep State #
// output - nothing
//----------------------------------------------------------------------
Method(SIOW, 1){
    Store("SIOW", Debug)

    //AMI_TODO: 
    ^ENFG(0x0A)                 //Set Logical Device 04 (PME)

    //1. Clear Status
    Store(^OPT1, PMFG)          // PMFG=PME  Wake Status
    Store(0xFF, ^OPT1)          //Clear I/O PME# Status
    And(^OPT0, 0xE7, ^OPT0)     //Clear KBC/Mouse PME Event

    //2. Disable PME
    Or(0x40, ^OPT2, Local0)        //Diable PME#, set bit 6
    Store(Local0, ^OPT2)
    ^EXFG()
}

//----------------------------------------------------------------------
// SIOH - SIO event handler, to be called from correspondent _Lxx method
// in order to serve the SIO chipset side of wake up event
//----------------------------------------------------------------------
// input  - nothing
// output - nothing
//----------------------------------------------------------------------
Method(SIOH, 0){
    #if IT8625_KEYBOARD_PRESENT
    If(And(PMFG, 0x08)){
        Notify(PS2K, 0x2)   //KBD Wake up
    }
    #endif
    #if IT8625_MOUSE_PRESENT
    If(And(PMFG, 0x10)){
        Notify(PS2M, 0x2)   //MOUSE Wake up
    }
    #endif
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************