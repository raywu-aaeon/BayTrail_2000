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
#ifndef __SIO_DXE_INIT__H__
#define __SIO_DXE_INIT__H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _EFI_DXE_PROTOCOL_DESCRIPTOR {
  EFI_GUID                     *Guid;
  VOID                         *Protocol;
  VOID                         *Reserved;
} EFI_DXE_PROTOCOL_DESCRIPTOR;

#if NPCE791x_SUPPORT
typedef struct _AMI_BOARD_INIT_LOCAL_PROTOCOL {
    UINTN      SioUid;
    UINTN      FunctionCount;
    AMI_BOARD_INIT_FUNCTION  Functions[10];
} AMI_BOARD_INIT_LOCAL_PROTOCOL;

AMI_BOARD_INIT_LOCAL_PROTOCOL SioInitLocalProtocol={
	0,	//Index / UID of SIO
	10,	//Actual number of function names entered
	Func0,	//0 Initial default function	
    KBC_Init,	//1 Function Name for Ps2Kb
	COM_Init,	//2 Function Name for Com1, SioIR
	CIR_Init,	//3 Function Name for SioCIR
	MSWC_Init,	//4 Function Name for MSWC
	SHM_Init,	//5 Function Name for SHM
	PM1_Init,	//6 Function Name for PM1
	PM2_Init,	//7 Function Name for PM2	
	PM3_Init,	//8 Function Name for PM3
	ESHM_Init,	//9 Function Name for ESHM
};

AMI_BOARD_INIT_PROTOCOL *gSioInitProtocolPtr=(AMI_BOARD_INIT_PROTOCOL*)&SioInitLocalProtocol;
#endif //NPCE791x_SUPPORT
//If have more SIO, should produce below information....
//typedef struct _AMI_BOARD_INIT_LOCAL_PROTOCOL1 {
//    UINTN      SioUid;
//    UINTN      FunctionCount;
//    AMI_BOARD_INIT_FUNCTION  Functions[1];
//} AMI_BOARD_INIT_LOCAL_PROTOCOL1;
//
//AMI_BOARD_INIT_LOCAL_PROTOCOL1 Sio1InitLocalProtocol={
//	1,	//Index / UID of SIO
//	1,	//Actual number of function names entered
//	Func1,	//0 Initial default function
////	FDC_Init	//1 Function Name for Fdcx
//};

EFI_DXE_PROTOCOL_DESCRIPTOR mSioProtocolList[] =  {
#if NPCE791x_SUPPORT
		{	&gAmiBoardSioInitProtocolGuid,
			&SioInitLocalProtocol,
			NULL
		},
#endif NPCE791x_SUPPORT
//If have more SIO, should produce below information....
//		{	&gAmiBoardSioInitProtocolGuid,
//			&Sio1InitLocalProtocol,
//			NULL
//		},
		{	NULL,
			NULL,
			NULL
		}
};
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
