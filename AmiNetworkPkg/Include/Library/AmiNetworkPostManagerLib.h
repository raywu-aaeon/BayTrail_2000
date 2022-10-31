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

//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	AmiNetworkPostManagerLib.h
//
// Description:	AmiNetworkPostManagerLib Definitions
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _AMI_NETWORK_POSTMANAGER_LIB_H_
#define _AMI_NETWORK_POSTMANAGER_LIB_H_

VOID 
AMICreatePopUp(
    IN  CHAR16          *String
);

VOID 
AMIPrintText(
    IN  CHAR16          *String
);

VOID 
AMISwitchToPostScreen (
);

VOID 
ClearGraphicsScreen (
 VOID
);

EFI_STATUS
CheckInvalidCharinIpAddress (
  IN CONST CHAR16       *IpSource,
  IN UINT8              IpMode
  );
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

