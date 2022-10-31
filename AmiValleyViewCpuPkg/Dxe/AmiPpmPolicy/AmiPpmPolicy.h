//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
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
// $Header: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/AmiPpmPolicy.h 1     11/23/10 2:00a Davidhsieh $
//
// $Revision: 1 $
//
// $Date: 11/23/10 2:00a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/AmiPpmPolicy.h $
// 
// 1     11/23/10 2:00a Davidhsieh
//
//**********************************************************************
#ifndef _AMI_PPM_POLICY_H_
#define _AMI_PPM_POLICY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	UINT8	TurboModeAvailable;
	UINT8	XETdcTdpLimitAvailable;
	UINT8	XECoreRatioLimitAvailable;
	UINT8	C3Available;
	UINT8	C6Available;
	UINT8	C7Available;
    UINT8   EISTAvailable;
    UINT8   TccActivationAvailable;
    UINT8   IsSandyBridge;
} SETUP_SNBPPM_FEATURES;

typedef struct {
	UINT16  NumberOfPStates;
	UINT16	Reserved0;
	UINT16	Reserved1;
	UINT16	Reserved2;
	UINT16	Reserved3;
}OEM_CPU_DATA;


/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
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