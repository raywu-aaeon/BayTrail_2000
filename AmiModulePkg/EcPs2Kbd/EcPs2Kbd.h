//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//
// $Header: /Alaska/SOURCE/Modules/EcPs2Kbd/EcPs2Kbd.h 3     7/18/11 5:58p Stacyh $
//
// $Revision: 3 $
//
// $Date: 7/18/11 5:58p $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:		EcPs2Kbd.h
//
// Description:	defines needed for EcPs2Kbd.
//
// Notes:		
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>
#include <GenericSIO.h>
#include <Protocol\DevicePath.h>
#include <AmiDxeLib.h>


typedef struct {
	ACPI_HID_DEVICE_PATH		AcpiDevicePath;
	EFI_DEVICE_PATH_PROTOCOL	EndDevicePath;
} PS2_KBD_DEVICE_PATH;


typedef struct _PS2KBD_DEV {
	EFI_DEVICE_PATH_PROTOCOL	*DevicePath;
	EFI_HANDLE					Handle;
} PS2KBD_DEV;

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
