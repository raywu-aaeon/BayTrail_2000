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
// $Header: /AptioV/Source/Modules/Ofbd/DefCfg/DefCfg.c $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	DefCfg.c
//
// Description:
// OFBD Default Command Configuration provides BIOS an oppertunity to override end user issued command in AFU.
//
// For example, BIOS could disable /k command even user issued it in AFU, or automatically enable /b when /p
// is issued.
//
// Please reference OFBDDEFCFGHandle for more details.
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include "DefCfg.h"
#include "../Ofbd.h"

//#define CONVERT_TO_STRING(a) #a
#define STR(a) CONVERT_TO_STRING(a)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	OFBDDEFCFGHandle
//
// Description:	OFBD Default Command Configuration Handle
//
// Input:
//      IN OUT OFBD_HDR *pOFBDHdr
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
OFBDDEFCFGHandle( 
    IN OUT OFBD_HDR *pOFBDHdr)
{
    EFI_STATUS Status = EFI_SUCCESS;

#if DEF_CFG_SAMPLE_TEST

	UINT8 *pOFBDTblEnd;
	OFBD_TC_51_DC_STRUCT *DCStructPtr;   
	
	pOFBDTblEnd = (UINT8 *)((UINT8 *)pOFBDHdr + (pOFBDHdr->OFBD_Size));
	DCStructPtr = (OFBD_TC_51_DC_STRUCT *)((UINT8 *)pOFBDHdr + pOFBDHdr->OFBD_HDR_SIZE + sizeof(OFBD_EXT_HDR)); 
	
	//
	// OEM add
	//
	
	// Sample : always cancel /K command
	if (DCStructPtr->ddRetSts & OFBD_TC_CFG_K)
	{
		DCStructPtr->ddExtCfg |= OFBD_TC_CFG_K;
	}
	
	// Sample : add new /B command when /P has issued
	if (DCStructPtr->ddRetSts & OFBD_TC_CFG_P)
	{
		DCStructPtr->ddRetSts |= OFBD_TC_CFG_B;
	}

    // Sample : For GAN command 
    // If ddRetSts equals to "0xFFFFFFFF", this means BIOS supply "/GAN" command.
	if (DCStructPtr->ddRetSts & OFBD_TC_CFG_GAN)
	{
		DCStructPtr->ddRetSts = 0xFFFFFFFF;
	}
#endif

    return(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	DEFCFGEntry
//
// Description:	OFBD Default Command Configuration Entry Point
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID DEFCFGEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled )
{
	OFBD_HDR *pOFBDHdr;
	OFBD_EXT_HDR *pOFBDExtHdr; 
	VOID *pOFBDTblEnd;
	OFBD_TC_51_DC_STRUCT *DCStructPtr;  

	if(*pOFBDDataHandled == 0)
	{
		pOFBDHdr = (OFBD_HDR *)Buffer;
		pOFBDExtHdr = (OFBD_EXT_HDR *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_HDR_SIZE));
		DCStructPtr = (OFBD_TC_51_DC_STRUCT *)((UINT8 *)pOFBDExtHdr + sizeof(OFBD_EXT_HDR)); 
		pOFBDTblEnd = (VOID *)((UINT8 *)Buffer + (pOFBDHdr->OFBD_Size));    
		
		if(pOFBDHdr->OFBD_FS & OFBD_FS_CFG)
		{   
			//Check Type Code ID
			if(pOFBDExtHdr->TypeCodeID == OFBD_EXT_TC_AFUDEFCFG)
			{  
				if(OFBDDEFCFGHandle(pOFBDHdr) == EFI_SUCCESS)
				{
					//OEM Default Command Configuration Handled.
					*pOFBDDataHandled = 0xFF;      
					return;
				}
			} 
			//Error occured
			*pOFBDDataHandled = 0xFE;          
			return;
		}  
	}

    return;
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
