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
// $Header: /Alaska/SOURCE/Modules/PTT/FastBootTseHook.c 6     6/01/12 6:56a Bibbyyeh $
//
// $Revision: 6 $
//
// $Date: 6/01/12 6:56a $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  FastBootTseHook.c
//
//  Description:
//  Implementation of suppress of TSE "Press DEL..." message
//
//<AMI_FHDR_END>
//*************************************************************************
//============================================================================
// Includes
//============================================================================
#include <Efi.h>
#include <Setup.h>
#include <Protocol/FastBootProtocol.h>
#include <FastBoot.h>
#include "AMITSEStrTokens.h"
//============================================================================
// Define
//============================================================================
#define	BOOT_FLOW_CONDITION_NORMAL	0

//============================================================================
// External Golbal Variable Declaration
//============================================================================
extern EFI_RUNTIME_SERVICES *gRT;
extern EFI_BOOT_SERVICES 	*gBS;
extern UINT32 gBootFlow;
extern BOOLEAN gEnterSetup;
extern EFI_EVENT gKeyTimer;
extern CHAR16 *HiiGetString( VOID* handle, UINT16 token );
extern EFI_HII_HANDLE  gHiiHandle;
//============================================================================
// External Function Definitions
//============================================================================
EFI_STATUS PostManagerDisplayPostMessage( CHAR16 *message );
BOOLEAN ProcessConInAvailability(VOID);
VOID BbsBoot(VOID);
VOID TSEIDEPasswordFreezeDevices(); 	//(EIP68329)+
VOID CheckForKeyHook( EFI_EVENT Event, VOID *Context );
EFI_STATUS TimerStopTimer( EFI_EVENT *Event );    
//============================================================================
// Golbal Variable Declaration
//============================================================================
FAST_BOOT_POLICY    *gFastBootPolicy;
FAST_BOOT_TSE_PROTOCOL gFastBootTseProtocol = {
    FastBootCheckForKey,
    FastBootStopCheckForKeyTimer,
    TSEIDEPasswordFreezeDevices         //(EIP68329)++
};
//============================================================================
// Function Definitions
//============================================================================
										//(EIP63924+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	FastBootCheckForKey
//
// Description:	This function check TSE variable,gEnterSetup and gBootFlow.
//              
// Input:		
//          BOOLEAN *EnterSetup
//          UINT32  *BootFlow
// Output:		
//          EFI_SUCCESS - Bootflow is changed or EnterSetup if true
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS 
FastBootCheckForKey ( 
  IN BOOLEAN *EnterSetup,
  IN UINT32  *BootFlow )
{
    CheckForKeyHook( (EFI_EVENT)NULL, NULL );

    *EnterSetup = gEnterSetup;
    *BootFlow = gBootFlow;
    
    if ((gEnterSetup) || (gBootFlow != BOOT_FLOW_CONDITION_NORMAL)){
        return EFI_SUCCESS;
    }
    else { 
    	return EFI_NOT_READY;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	FastBootStopCheckForKeyTimer
//
// Description:	This function stop the timer of CheckForKey callback
//
// Input:		None
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>  

EFI_STATUS 
FastBootStopCheckForKeyTimer()
{
    return TimerStopTimer(&gKeyTimer);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootMinisetupDriverEntryHook
//
// Description: Function that will be called when enter TSE Dxe entry
//
// Input:       None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBootMinisetupDriverEntryHook(VOID)
{
    EFI_HANDLE Handle = NULL;			
    EFI_GUID   FastBootTseGuid = FAST_BOOT_TSE_PROTOCOL_GUID; 
    EFI_GUID   FastBootPolicyGuid = FAST_BOOT_POLICY_PROTOCOL_GUID;
    EFI_STATUS Status;
    
    gBS->InstallProtocolInterface(&Handle,
                              &FastBootTseGuid,
                              EFI_NATIVE_INTERFACE,
                              &gFastBootTseProtocol);

    Status = gBS->LocateProtocol(&FastBootPolicyGuid,NULL,&gFastBootPolicy);
        
} 
										//<(EIP63924+)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootConInAvailHook
//
// Description: Function that will be called instead of generic TSE callback
//              on Console Input device is installed event
//
// Input:		None
//
// Output:  
//  BOOLEAN -   Return TRUE if the screen was used to ask password; FALSE if 
//              the screen was not used to ask password.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
                                 
BOOLEAN 
FastBootConInAvailHook(VOID)
{

	if(gFastBootPolicy->FastBootEnable == 0 || gFastBootPolicy->CheckPassword == 1) 
        return ProcessConInAvailability();

#if SETUP_PRINT_ENTER_SETUP_MSG 
{
        CHAR16 *text = NULL;
        text = HiiGetString( gHiiHandle, STRING_TOKEN(STR_DEL_ENTER_SETUP) );
        if ( text != NULL )
            PostManagerDisplayPostMessage(text);
        gBS->FreePool(text);
}
#endif

    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootBbsBootHook
//
// Description: Function that will be called instead of generic TSE callback
//              on BBS popup boot path
//
// Input:		None
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FastBootBbsBootHook(VOID)
{
    static EFI_GUID FastBootVariableGuid = FAST_BOOT_VARIABLE_GUID;
    EFI_STATUS Status;
    UINT32     BbsPopupCalled = 0x55aa55aa;
    Status = gRT->SetVariable(L"BbsPopupCalled", 
                              &FastBootVariableGuid, 
                              EFI_VARIABLE_BOOTSERVICE_ACCESS,
                              sizeof(BbsPopupCalled),
                              &BbsPopupCalled);
    BbsBoot();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootLaunch
//
// Description: Function that will be called instead of generic TSE callback
//              on fast boot path
//
// Input:		
//  None
//
// Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

#if OVERRIDE_FastBootLaunch
EFI_STATUS 
FastBootLaunch()
{
	EFI_STATUS Status = EFI_UNSUPPORTED;
	AMI_FAST_BOOT_PROTOCOL * FastBootProtocol = NULL;

	// do the Fast Boot
	if (!EFI_ERROR(gBS->LocateProtocol(&gAmiFastBootProtocolGuid, NULL, &FastBootProtocol)))
		Status = FastBootProtocol->Launch();

	// If gFastBootProtocolGuid protocol not found or FastBoot Failed. return
	return Status;
}
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
