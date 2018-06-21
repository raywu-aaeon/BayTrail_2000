//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TCGSmm/TCGSmm.h 5     8/09/11 6:28p Fredericko $
//
// $Revision: 5 $
//
// $Date: 8/09/11 6:28p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name: TCGSmm.h
//
// Description:
// Header file for TCGSMM subcomponent
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _TCGSMM_H_
#define _TCGSMM_H_

//#include <AmiDxeLib.h>
//#include <Protocol\SmmThunk.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\DevicePath.h>
#include <Protocol\LoadedImage.h>
#include <Protocol\SmmSwDispatch.h>
#include <Protocol\SmmSwDispatch2.h>
#include <Token.h>
//#include <AmiDxeLib.h>
#include <HOB.h>
#include <AmiTcg\TCGMisc.h>
#include <AmiTcg\tcg.h>


EFI_GUID        SmmtcgefiOsVariableGuid    = AMI_TCG_EFI_OS_VARIABLE_GUID;
EFI_GUID        SmmFlagsStatusguid         = AMI_TCG_CONFIRMATION_FLAGS_GUID;
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2005, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
