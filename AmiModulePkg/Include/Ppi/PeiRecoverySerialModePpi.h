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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: PeiRecoverySerialModePpi.h - PPI definitions for RecoverySerialMode
//
// Description:	Defines GUIDs and structs that make up the RecoverySerialModePpi
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _PEI_RECOVERY_SERIAL_MODE_PPI_H_
#define _PEI_RECOVERY_SERIAL_MODE_PPI_H_

#define PEI_RECOVERY_SERIAL_MODE_PPI_GUID \
  { 0x5e7063d3, 0xc12, 0x475b, 0x98, 0x35, 0x14, 0xab, 0xb1, 0xcb, 0xe, 0xe9 }

typedef struct {
	UINT16  SerialDeviceBaseAddress;
} PEI_RECOVERY_SERIAL_MODE_PPI;

extern EFI_GUID gSerialRecoveryDevicePpiGuid;

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
