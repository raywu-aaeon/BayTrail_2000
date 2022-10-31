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
// Name:        EcPs2Ms.ASL
//
// Description: Define ACPI methods for EC Ps2 Mouse device 
//
//*************************************************************************
//<AMI_FHDR_END>

//**********************************************************************;
// PS2 Mouse Device, IO category # - 12
//---------------------------------------------------------------------

Scope (\_SB.PCI0) {
    Device(PS2M) {
        Name(_HID, EISAID("PNP0F03")) // Hardware Device ID - Microsoft mouse
                                // check if MSFT Mouse driver supports D3 properly on all MSFT OSes. 
                                // It may prevent OS to go to S3 sleep state
                                // Use Logitech _HID instead if OS rejecting to go to S3
        Name(_UID, 0x00)        // Unique ID.

        Method(_STA, 0) {
            Return(0x0f)
        }

        Name(CRS1, ResourceTemplate() {
            IRQNoFlags(){12}
        })

        Name(CRS2, ResourceTemplate() {
            IO(Decode16, 0x60, 0x60, 0, 0x1)
            IO(Decode16, 0x64, 0x64, 0, 0x1)
            IRQNoFlags(){12}
        })

        Method(_CRS,0) {
            If(And(\IOST, 0x0400)) { 
                // PS2K is present, I/O resources 0x60 & 0x64 are reserved there
                Return(CRS1)
            } else {            
                // single PS/2 mouse(no PS/2 kbd), need to supply I/O resources 0x60 & 0x64 for PS/2 Controller.
                Return(CRS2)
            }
        }

        Name(_PRS, ResourceTemplate(){
            StartDependentFn(0, 0) { 
                IRQNoFlags(){12} 
            }
            EndDependentFn()
        })

        // Mouse 2nd Level wake up control method
        Method(_PSW, 1) {
            Store(Arg0, \MSFG)
        }
    }// End of PS2M

    Scope(\) {
        Name(\MSFG, 0x01)   //Mouse wake-up flag default enable
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
