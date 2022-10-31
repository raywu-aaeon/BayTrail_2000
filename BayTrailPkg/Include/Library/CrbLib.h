//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**   **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: /Alaska/BIN/Modules/CRB Board/CRBLib.h 3     2/19/09 10:02p Abelwu $
//
// $Revision: 3 $
//
// $Date: 2/19/09 10:02p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/BIN/Modules/CRB Board/CRBLib.h $
// 
// 3     2/19/09 10:02p Abelwu
// Updated for AMI Coding Standard v0.99
// 
// 2     6/04/08 6:04a Abelwu
// Updated the header of the source file.
// 
// 1     6/03/08 2:40a Abelwu
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CrbLib.h
//
// Description: Custom Reference Board (or Demo Board) header file.
//              Defines all the CRB specific equates and structures in
//              this file. 
//
//<AMI_FHDR_END>
//*************************************************************************

#ifndef __CRBLIB_H__
#define __CRBLIB_H__

//---------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PCILIB_TO_COMMON_ADDRESS
#define PCILIB_TO_COMMON_ADDRESS(Address) \
	((UINT64) ((((UINTN) ((Address>>20) & 0xff)) << 24) + (((UINTN) ((Address>>15) & 0x1f)) << 16) + (((UINTN) ((Address>>12) & 0x07)) << 8) + ((UINTN) (Address & 0xfff ))))
#endif

// Dummy verb table
UINT32 mAzaliaVerbTableDataDummy[] = {
		  0x00871F01,
		  0x00871E44,
		  0x00871D21,
		  0x00871C00,
		  0x00971F01,
		  0x00971EC4,
		  0x00971D21,
		  0x00971C10,
		  0x00B71F01,
		  0x00B71E01,
		  0x00B71D40,
		  0x00B71C20,
		  0x00E71F01,
		  0x00E71E01,
		  0x00E71D40,
		  0x00E71C21
};


#ifdef __cplusplus
}
#endif
#endif


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
