  Method(ADBG, 1, Serialized)
  {

    Return(0)
  }
  Name(WAKP, Package(){Zero, Zero})

  //
  //EIP143681 >>
  //
  // The following ASL use to clear XHCI PME_STS and XHCI PME_EN bit.
  // Put the ASL code in the ACPI method which will be called after S3 resume or
  // polling in OS.
  //
  Method(PMED,0)
  {
    Store(1, \_SB.PCI0.XHC1.PMES) //Clear PME status
    Store(0, \_SB.PCI0.XHC1.PMEE) //Disable PME
  }
  //EIP143681 <<

/*------------------------------------------------------------------------
;-------------------------------------------------------------------------
;
; Procedure:	GPRW
;
; Description:	Generic Wake up Control Method ("Big brother") to detect the Max Sleep State available in ASL Name scope
;		and Return the Package compatible with _PRW format.
; Input: Arg0 =  bit offset within GPE register space device event will be triggered to
;        Arg1 =  Max Sleep state, device can resume the System from
;             If Arg1 = 0 , Update Arg1 with Max _Sx state enabled in the System
; Output: _PRW package
;
;-------------------------------------------------------------------------
--------------------------------------------------------------------------*/
#if 1
Name(PRWP, Package(){Zero, Zero})		// _PRW Package
Method(GPRW, 2)
{
	Store(Arg0, Index(PRWP, 0))		// copy GPE#
// SS1-SS4 - enabled in BIOS Setup Sleep states
    Store(ShiftLeft(SS1,1),Local0)      	// S1 ?
    Or(Local0,ShiftLeft(SS2,2),Local0)      // S2 ?
    Or(Local0,ShiftLeft(SS3,3),Local0)      // S3 ?
    Or(Local0,ShiftLeft(SS4,4),Local0)      // S4 ?
// Local0 has a bit mask of enabled Sx(1 based)
// bit mask of enabled in BIOS Setup Sleep states(1 based)
	If(And(ShiftLeft(1, Arg1), Local0)){	// Requested wake up value (Arg1) is present in Sx list of available Sleep states
		Store(Arg1, Index(PRWP, 1))	// copy Sx#
	} Else {  				// Not available -> match Wake up value to the higher Sx state
		ShiftRight(Local0, 1, Local0)	
//			If(LOr(LEqual(OSFL, 1), LEqual(OSFL, 2))) { 	// ??? Win9x
//				FindSetLeftBit(Local0, Index(PRWP,1))	// Arg1 == Max Sx
//			} Else { 					// ??? Win2k / XP
        		FindSetRightBit(Local0, Index(PRWP,1))	// Arg1 == Min Sx
//			}
	}

	Return(PRWP)
}
#endif