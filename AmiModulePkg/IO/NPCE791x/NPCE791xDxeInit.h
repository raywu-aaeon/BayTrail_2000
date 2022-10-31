//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
// $Header: /Alaska/Tools/template.h 6     1/13/10 2:13p Felixp $
//
// $Revision: 6 $
//
// $Date: 1/13/10 2:13p $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/Tools/template.h $
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
#ifndef __NPCE791x_DXE_INIT__H__
#define __NPCE791x_DXE_INIT__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <AmiDxeLib.h>
#include <Token.h>
#include <GenericSIO.h>
#include <Setup.h>
#include <Protocol\AmiSio.h>
#include <Protocol\PciIo.h>
#include <Library\AmiSioDxeLib.h>

//#include "..\Include\SioCommon.h"

EFI_STATUS	Func0(
		AMI_BOARD_INIT_PROTOCOL		*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
);

EFI_STATUS KBC_Init(
	AMI_BOARD_INIT_PROTOCOL	*This,
	IN UINTN					*Function,
	IN OUT VOID					*ParameterBlock
);

EFI_STATUS COM_Init(
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
);

EFI_STATUS CIR_Init(
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
);

EFI_STATUS MSWC_Init(
	AMI_BOARD_INIT_PROTOCOL	*This,
	IN UINTN					*Function,
	IN OUT VOID					*ParameterBlock
);

EFI_STATUS SHM_Init(
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
);

EFI_STATUS PM1_Init(
	AMI_BOARD_INIT_PROTOCOL	*This,
	IN UINTN					*Function,
	IN OUT VOID					*ParameterBlock
);

EFI_STATUS PM2_Init(
		AMI_BOARD_INIT_PROTOCOL	*This,
		IN UINTN					*Function,
		IN OUT VOID					*ParameterBlock
);

EFI_STATUS PM3_Init(
	AMI_BOARD_INIT_PROTOCOL	*This,
	IN UINTN					*Function,
	IN OUT VOID					*ParameterBlock
);

EFI_STATUS ESHM_Init(
	AMI_BOARD_INIT_PROTOCOL	*This,
	IN UINTN					*Function,
	IN OUT VOID					*ParameterBlock
);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************



