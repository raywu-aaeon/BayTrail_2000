//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2011, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
// $Archive: /Alaska/BIN/Modules/AMITSE2_0/AMITSE/bootflow.c $
//
// $Author: Rajashakerg $
//
// $Revision: 17 $
//
// $Date: 9/17/12 5:59a $
//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:		bootflow.c
//
// Description:	This file bootflow related functions and tables
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifdef TSE_FOR_APTIO_4_50

#include "Token.h"
#include <Efi.h>
#include <Protocol/SimpleTextIn.h>

#include "Setup.h" // for MAIN_FORM_SET_CLASS
#include "AutoId.h" // for MAIN_MAIN

#else //#ifdef TSE_FOR_APTIO_4_50

#include "minisetup.h"

#endif //#ifdef TSE_FOR_APTIO_4_50

#include "AMIVfr.h"
#include "variable.h"
#include "boot.h"
#include "bootflow.h"
#include "mem.h"
#include "commonoem.h"
#include "HookAnchor.h"

#ifndef MAIN_FORM_SET_CLASS
#define MAIN_FORM_SET_CLASS 1
#endif

#ifndef MAIN_MAIN
#define MAIN_MAIN	1
#endif

extern BOOLEAN gEnterSetup;
extern EFI_GUID gEfiShellFileGuid;

EFI_GUID _gBootFlowGuid = BOOT_FLOW_VARIABLE_GUID;
static EFI_GUID _gTPMBootFlowGuid = TPM_BOOT_FLOW_VARIABLE_GUID;

VOID SetupDebugPrint(IN CONST CHAR8  *Format, ...) ;
#if SUPPRESS_PRINT
    #define SETUP_DEBUG_TSE(format,...)
#else //Else of SUPPRESS_PRINT
#if TSE_GNUC_BUILD_SUPPORT
    #define SETUP_DEBUG_TSE(format,...) SetupDebugPrint(format, ##__VA_ARGS__)
#else //Else of TSE_GNUC_BUILD_SUPPORT
    #define SETUP_DEBUG_TSE(format,...) SetupDebugPrint(format, __VA_ARGS__)
#endif //End of TSE_GNUC_BUILD_SUPPORT
#endif

static BOOT_FLOW _gBootFlowTable[] =
{
//	{ Condition,						            PageClass,				PageSubClass,	PageFormID,	ControlNumber,	MessageBoxToken,	MessageBoxTimeout,	GotoPageOnEntry,	ProceedBooting,	InfiniteLoop,	                            LaunchShell,	DoNotEnterSetup,	CallbackFunction },
    { BOOT_FLOW_CONDITION_NORMAL,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,	        BOOT_FLOW_NORMAL_INFINITE_LOOP,	            BOOT_FLOW_NORMAL_LAUNCH_DEFAULT_BOOTIMAGE,  FALSE,				NULL },
	{ BOOT_FLOW_CONDITION_ERROR,		            ERROR_MANAGER_KEY_ID,	0,				1,			0,				0,					0,					TRUE,				FALSE,			FALSE,			                            TRUE,			                            FALSE,				NULL },
	{ BOOT_FLOW_CONDITION_RECOVERY,		            0x40,					0,				1,			0,				0,					0,					TRUE,				TRUE,			FALSE,			                            TRUE,			                            FALSE,				NULL },
	{ BOOT_FLOW_CONDITION_PCI_OUT_OF_RESOURCE,		0x79,		            0,				1,			0,				0,					0,					TRUE,				TRUE,			FALSE,			                            TRUE,			                            FALSE,				NULL },
	{ BOOT_FLOW_CONDITION_FIRST_BOOT,	            MAIN_FORM_SET_CLASS,	0,				MAIN_MAIN,	0,				0,					0,					TRUE,				TRUE,			FALSE,			                            TRUE,			                            FALSE,				NULL },
	{ BOOT_FLOW_CONDITION_OS_UPD_CAP,	            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&OsUpdateCapsuleWrap },		//EIP93521
    { BOOT_FLOW_HOTKEY_BOOT,                        0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&LaunchHotKeyBootOption },
#if SETUP_OEM_KEY1_ENABLE
	{ BOOT_FLOW_CONDITION_OEM_KEY1,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&OemKey1HookHook },
#endif
#if SETUP_OEM_KEY2_ENABLE
	{ BOOT_FLOW_CONDITION_OEM_KEY2,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&OemKey2HookHook },
#endif
#if SETUP_OEM_KEY3_ENABLE
	{ BOOT_FLOW_CONDITION_OEM_KEY3,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&OemKey3HookHook },
#endif
#if SETUP_OEM_KEY4_ENABLE
	{ BOOT_FLOW_CONDITION_OEM_KEY4,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&OemKey4HookHook },
#endif
#if SETUP_BBS_POPUP_ENABLE
	{ BOOT_FLOW_CONDITION_BBS_POPUP,	            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				&DoPopup },
#endif
	{ BOOT_FLOW_CONDITION_OEM_KEY_CALLBACK,	        0,					    0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            FALSE,			                            FALSE,				NULL }, // Callback is null and on OEMkey press it will updated.
	{ BOOT_FLOW_CONDITION_NO_SETUP,		            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			TRUE,			                            FALSE,			                            TRUE,				NULL },
#if FAST_BOOT_SUPPORT
	{ BOOT_FLOW_CONDITION_FAST_BOOT,	            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            TRUE,			                            TRUE,				&FBBootFlow },
#endif
	// this MUST be the last entry in the boot flow table
	{ BOOT_FLOW_CONDITION_NULL,			            0,						0,				0,			0,				0,					0,					FALSE,				TRUE,			FALSE,			                            TRUE,			                            FALSE,				NULL }
};

// Set the initial Bootflow table.
// OEM may have thier own Bottflow table and update the gBootFlowTable with that in the Entry hooks.
BOOT_FLOW	*gBootFlowTable = _gBootFlowTable;
EFI_STATUS LaunchSecondaryBootPath (CHAR16 *);			//EIP 88447

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	BootFlowManageEntry
//
// Description:	Determines boot flow entry path based on the boot
//              flow variable.
//
// Input:		VOID
//
// Output:		EFI_SUCCESS: Normal boot
//              EFI_UNSUPPORTED: do not proceed to boot
//              EFI_NOT_STARTED: Do not enter setup
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS BootFlowManageEntry( VOID )
{
	EFI_STATUS Status = EFI_SUCCESS;
	UINT32 condition = BOOT_FLOW_CONDITION_NORMAL;
	UINT32 *conditionPtr;
	UINTN size = 0;

	BOOT_FLOW *bootFlowPtr;

	SETUP_DEBUG_TSE ("\n[TSE] Entering BootFlowManageEntry");
	conditionPtr = VarGetNvramName( L"BootFlow", &_gBootFlowGuid, NULL, &size );
	if ( conditionPtr != NULL )
		condition = *conditionPtr;

	MemFreePointer( (VOID **)&conditionPtr );

	size = 0;
    conditionPtr = VarGetNvramName( L"TPMBootFlow", &_gTPMBootFlowGuid, NULL, &size );

    if ( conditionPtr != NULL )
    {
        if(*conditionPtr != BOOT_FLOW_CONDITION_NORMAL)
            condition = *conditionPtr;
    }

	MemFreePointer( (VOID **)&conditionPtr );
	bootFlowPtr = gBootFlowTable;
	for ( bootFlowPtr = gBootFlowTable;
			bootFlowPtr->Condition != BOOT_FLOW_CONDITION_NULL; bootFlowPtr++ )
	{
		if ( condition != bootFlowPtr->Condition )
			continue;
		SETUP_DEBUG_TSE ("\n[TSE] BootFlow condition is %d", condition);
		if ( bootFlowPtr->CallbackFunction != NULL )
			Status = bootFlowPtr->CallbackFunction( bootFlowPtr );
		if ( ! bootFlowPtr->ProceedBooting )
			Status = EFI_UNSUPPORTED;

		if ( bootFlowPtr->MessageBoxToken != 0 )
			_BootFlowShowMessageBox( bootFlowPtr );
		if ( ( bootFlowPtr->GotoPageOnEntry ) && ( bootFlowPtr->PageClass != 0 ) )
		{
			_BootFlowSetActivePage( bootFlowPtr );
			_BootFlowSetActiveControl( bootFlowPtr );
		}

		if(bootFlowPtr->DoNotEnterSetup)
		{
			gEnterSetup = FALSE;
			Status = EFI_NOT_STARTED;
		}

		break;
	}
	SETUP_DEBUG_TSE ("\n[TSE] Exiting BootFlowManageEntry");
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	BootFlowManageExit
//
// Description:	Determines boot flow exit path based on the boot
//              flow variable.
//
// Input:		VOID
//
// Output:		EFI_UNSUPPORTED: enter setup if all the boot options
//              fail
//              EFI_NOT_STARTED: try again and again if all boot
//              options fail
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS BootFlowManageExit (VOID)
{
	EFI_STATUS Status = EFI_SUCCESS;
	UINT32 condition = BOOT_FLOW_CONDITION_NORMAL;
	UINT32 *conditionPtr;
	UINTN size = 0;

	BOOT_FLOW *bootFlowPtr;
	SETUP_DEBUG_TSE ("\n[TSE] Entering BootFlowManageExit");
	conditionPtr = VarGetNvramName( L"BootFlow", &_gBootFlowGuid, NULL, &size );
	if ( conditionPtr != NULL )
		condition = *conditionPtr;

	MemFreePointer( (VOID **)&conditionPtr );

	size =0;
    conditionPtr = VarGetNvramName( L"TPMBootFlow", &_gTPMBootFlowGuid, NULL, &size );

	if ( conditionPtr != NULL )
	{
		if(*conditionPtr != BOOT_FLOW_CONDITION_NORMAL)
		condition = *conditionPtr;
	}

	MemFreePointer( (VOID **)&conditionPtr );

	bootFlowPtr = gBootFlowTable;
	for ( bootFlowPtr = gBootFlowTable;
			bootFlowPtr->Condition != BOOT_FLOW_CONDITION_NULL; bootFlowPtr++ )
	{
		if ( condition != bootFlowPtr->Condition )
			continue;
		SETUP_DEBUG_TSE ("\n[TSE] BootFlow condition is %d", condition);

		if ( bootFlowPtr->PageClass != 0 )
		{
			_BootFlowSetActivePage( bootFlowPtr );
			_BootFlowSetActiveControl( bootFlowPtr );
			Status = EFI_NOT_STARTED;
			break;
		}

		if ( ! bootFlowPtr->InfiniteLoop )
			Status = EFI_UNSUPPORTED;
		else
			Status = EFI_NOT_STARTED;

		if (bootFlowPtr->LaunchShell)                 
		{
			LaunchSecondaryBootPath (gBootFileName);			//EIP 88447
			BootLaunchGuid (&gEfiShellFileGuid);
		}
		break;
	}
	SETUP_DEBUG_TSE ("\n[TSE] Exiting BootFlowManageExit");
	return Status;
}

//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2011, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
