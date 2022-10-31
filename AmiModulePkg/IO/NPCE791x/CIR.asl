//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
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
// $Header: /Alaska/BIN/IO/ITE/NPCE791xF/NPCE791x Device ASL Files/CIR.asl 2     9/06/11 10:49p Ruidai $
//
// $Revision: 2 $
//
// $Date: 9/06/11 10:49p $
//**********************************************************************;
// Revision History
// ----------------
// $Log: /Alaska/BIN/IO/ITE/NPCE791xF/NPCE791x Device ASL Files/CIR.asl $
// 
// 2     9/06/11 10:49p Ruidai
// Update to new templet.
// 
// 1     4/06/10 11:14p Mikes
// Initial check-in
//
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  <CIR.ASL>
//
// Description: Define ACPI method or namespce For Super IO
//
//<AMI_FHDR_END>
//*************************************************************************
// CIR //
// Category # :0x12 (Generic IO range 3)
//Device(CIR) {
//	Name(_HID, EISAID("PNP0510"))		//PnP Device ID IrDA
//	Name(_UID, 1)				//UID for IrDa 

	Method(_STA, 0) {Return(^^SIO1.DSTA(0x10))}	//Get CIR status
	Method(_DIS, 0) {^^SIO1.DCNT(0x10, 0)} 	//Disable CIR
	Method(_CRS, 0) {Return(^^SIO1.DCRS(0x10, 0))}	//Get CIR current resources
	Method(_SRS, 1) {^^SIO1.DSRS(Arg0, 0x10) }	//Set CIR recources
//	Method(_PRS, 0) {Return(^^SIO1.CMPR)}		//Return possible resources


//-----------------------------------------------------------------------
// IrDa Possible Resources
//-----------------------------------------------------------------------
//NOTE: _PRS MUST be the NAME not a METHOD object 
//to have GENERICSIO.C working right! 
//-----------------------------------------------------------------------
	Name(_PRS, ResourceTemplate() {
		StartDependentFn(0, 0) {
			IO(Decode16, 0x3E0, 0x3E0, 1, 8)
			IRQNoFlags() {10}
		}
		StartDependentFnNoPri() {
			IO(Decode16, 0x3E0, 0x3E0, 1, 8)
			IRQNoFlags() {3,4,5,6,7,9,10,11,12}
		}
		EndDependentFn()
    })
//} // End Of IRDA //
//-----------------------------------------------------------------------

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2008, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

