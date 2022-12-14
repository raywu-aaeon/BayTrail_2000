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

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: PeiRamBootOfbdLib.h
//
// Description: Definition file for PeiRamBootOfbdLib.
//
//<AMI_FHDR_END>
//**********************************************************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PeiRamBootOfbdEntry
//
// Description:	
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
PeiRamBootOfbdEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled
);
