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

//<AMI_FHDR_START>
//*************************************************************************
// Name:  		EcPs2Kb.ASL
//
// Description: Define ACPI methods for EC Ps2 Keyboard device 
//
//*************************************************************************
//<AMI_FHDR_END>


//**********************************************************************;
// PS2 Keyboard Device, IO category # - 10
//---------------------------------------------------------------------

Scope (\_SB.PCI0) {
    Device(PS2K) {
        Name(_HID,EISAID("PNP0303"))        // Standard Keyboard 101/102
        Name(_UID, 0x00)  					// Unique ID.

        Method(_STA,0) {
            Return (0x0F)
        }

        Name(_CRS,ResourceTemplate() {
            IO(Decode16, 0x60, 0x60, 0, 0x1)    //PS2 resource
       	    IO(Decode16, 0x64, 0x64, 0, 0x1)
            IRQNoFlags(){1}
        })

        Name(_PRS, ResourceTemplate() {
            StartDependentFn(0, 0) {  
                IO(Decode16, 0x60, 0x60, 0, 0x1)  
                IO(Decode16, 0x64, 0x64, 0, 0x1)
                IRQNoFlags(){1}
            }
        EndDependentFn()
        })

        // Keyboard 2nd Level wake up control method
        Method(_PSW, 1) {
            Store(Arg0, \KBFG)
        }
    }// End of PS2K

    Scope(\){
        Name(\KBFG, 0x01)   //Keyboard wake-up flag default enable
    }
} // End of \_SB.PCI0

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
