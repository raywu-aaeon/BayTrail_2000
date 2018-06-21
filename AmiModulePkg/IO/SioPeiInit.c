//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/Tools/template.c 6     1/13/10 2:13p Felixp $
//
// $Revision: 6 $
//
// $Date: 1/13/10 2:13p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/Tools/template.c $
// 
// 6     1/13/10 2:13p Felixp
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  <This File's Name>
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiPeiLib.h>

#include <Pei.h>
#include <Setup.h>
#include <AmiLib.h>
#include <Protocol\AmiSio.h>
#include <Library\AmiSioPeiLib.h>
//#include <Library\SioLibExt.h>

//-------------------------------------------------------------------------
// internal funtion declare; these funtions are only used by this file.
//-------------------------------------------------------------------------
//EIP144758 >>
IO_DECODE_DATA SioPeiDecodeTable[]={
    #include "build\PrivateSioPeiInitTable.h"
};

SIO_DEVICE_INIT_DATA SioPeiInitTable[]={
	{0x80, 0x00, 0xD0},
    #include "build\PrivateSioPeiInitTable.h"
};
//EIP144758 <<

//-------------------------------------------------------------------------
// global funtion declare ; these funtions are used for other files.
//-------------------------------------------------------------------------

//*************************************************************************
// belows are functions defined
//*************************************************************************

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: SioPeiInitEntryPoint
//
// Description:
//  This function provide PEI phase SIO initialization
//
// Input:
//  IN  EFI_FFS_FILE_HEADER    *FfsHeader - Logical Device's information
//  IN  EFI_PEI_SERVICES       **PeiServices  - Read/Write PCI config space
//
// Output:    None
//
// Modified:  Nothing
//
// Referrals: None
//
// Note:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STATUS SioPeiInitEntryPoint(
		IN       EFI_PEI_FILE_HANDLE  FileHandle,
		IN CONST EFI_PEI_SERVICES     **PeiServices
)
{
	UINT8 index;

	for(index=0; index<sizeof(SioPeiDecodeTable)/sizeof(IO_DECODE_DATA); index++)
		AmiSioLibSetLpcDeviceDecoding(NULL, SioPeiDecodeTable[index].BaseAdd, SioPeiDecodeTable[index].UID, SioPeiDecodeTable[index].Type);

	ProgramRtRegisterTable(0, SioPeiInitTable, sizeof(SioPeiInitTable)/sizeof(SIO_DEVICE_INIT_DATA));

	return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
