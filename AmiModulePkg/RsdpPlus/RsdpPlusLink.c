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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  		RsdpPlusLink.c
//
// Description:	E-link functions.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <AmiDxeLib.h>
#include "ShadowRamProtocol.h"

extern EFI_BOOT_SERVICES *gBS;
extern EFI_SYSTEM_TABLE *gST;
extern EFI_RUNTIME_SERVICES *gRT;

EFI_GUID gShdowRamProtocolGuid = SHADOW_RAM_PROTOCOL_GUID;
SHADOW_RAM_PROTOCOL	*gShadowRamProtocol;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	UpdateShadow
//
// Description: BeforeEfiBootLaunchHook eLink function.
//				
// Input:	VOID
//
// Output:	VOID
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateShadow(VOID)
{
	EFI_STATUS	Status;
	
	Status = gBS->LocateProtocol(&gShdowRamProtocolGuid,NULL,&gShadowRamProtocol);
    if(!EFI_ERROR(Status))
	    gShadowRamProtocol->UpdateShadowBeforEfiBoot();
		
	return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	EraseShadow
//
// Description: AfterEfiBootLaunchHook eLink function.
//				
// Input:	VOID
//
// Output:	VOID
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID EraseShadow(VOID)
{
	EFI_STATUS	Status;

	Status = gBS->LocateProtocol(&gShdowRamProtocolGuid,NULL,&gShadowRamProtocol);
    if(!EFI_ERROR(Status))
	    gShadowRamProtocol->EraseShadowAfterEfiBoot();
	
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
