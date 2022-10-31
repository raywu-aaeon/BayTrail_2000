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
// $Archive: /Alaska/BIN/Modules/AMITSE2_0/AMITSE/Inc/variable.h $
//
// $Author: Arunsb $
//
// $Revision: 9 $
//
// $Date: 1/30/12 1:33a $
//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:		variable.h
//
// Description:	Variable handling header
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef	_VARIABLE_H_
#define	_VARIABLE_H_

typedef struct _NVRAM_VARIABLE
{
	UINT8	*Buffer;
	UINTN	Size;
}
NVRAM_VARIABLE;

#define VAR_ZERO_OFFSET         0

#define	VAR_COMMAND_GET_VALUE	0
#define	VAR_COMMAND_SET_VALUE	1

#define	VAR_COMMAND_GET_NVRAM	0
#define	VAR_COMMAND_SET_NVRAM	1

// Variable IDs
#define VARIABLE_ID_SETUP			    		0
#define VARIABLE_ID_LANGUAGE		    		1		
#define VARIABLE_ID_BOOT_TIMEOUT	   		2
#define VARIABLE_ID_USER_DEFAULTS	   	3
#define VARIABLE_ID_ERROR_MANAGER	   	4
#define VARIABLE_ID_AMITSESETUP       		5
#define VARIABLE_ID_IDE_SECURITY      		6
#define VARIABLE_ID_BOOT_ORDER         	7
#define VARIABLE_ID_BBS_ORDER           	8
#define VARIABLE_ID_DEL_BOOT_OPTION     	9
#define VARIABLE_ID_ADD_BOOT_OPTION     	10
#define VARIABLE_ID_BOOT_MANAGER        	11
#define VARIABLE_ID_BOOT_NOW            	12
#define VARIABLE_ID_LEGACY_DEV_INFO     	13
#define VARIABLE_ID_AMI_CALLBACK        	14
#define VARIABLE_ID_LEGACY_GROUP_INFO   	15
#define VARIABLE_ID_OEM_TSE_VAR		    	17
#define VARIABLE_ID_DYNAMIC_PAGE_COUNT		18
#define VARIABLE_ID_DRV_HLTH_ENB				19
#define VARIABLE_ID_DRV_HLTH_COUNT			20
#define VARIABLE_ID_DRIVER_MANAGER			22		//EIP70421 & 70422 Support for driver order
#define VARIABLE_ID_DRIVER_ORDER				23	
#define VARIABLE_ID_ADD_DRIVER_OPTION   	24
#define VARIABLE_ID_DEL_DRIVER_OPTION   	25
#define VARIABLE_ID_PORT_OEM1					26		//EIP74676 variables for porting purpose
#define VARIABLE_ID_PORT_OEM2					27
#define VARIABLE_ID_PORT_OEM3					28
#define VARIABLE_ID_PORT_OEM4					29
#define VARIABLE_ID_PORT_OEM5					30

//EIP 76381 :  Performance Improving of variable data load and usage
EFI_STATUS GetNvramVariableList(NVRAM_VARIABLE **RetNvramVarList);
VOID CleanTempNvramVariableList();
EFI_STATUS CopyNvramVariableList(NVRAM_VARIABLE *SrcVarList, NVRAM_VARIABLE **DestVarList);

EFI_STATUS VarLoadVariables( VOID **list, NVRAM_VARIABLE *defaultList );
EFI_STATUS VarBuildDefaults( VOID );
VOID *VarGetNvramName( CHAR16 *name, EFI_GUID *guid, UINT32 *attributes, UINTN *size );
EFI_STATUS VarSetNvramName( CHAR16 *name, EFI_GUID *guid, UINT32 attributes, VOID *buffer, UINTN size );
VOID *VarGetNvram( UINT32 variable, UINTN *size );
VOID *VarGetNvramQuestionValue(UINT32 variable, UINTN Offset, UINTN Size);
EFI_STATUS VarSetNvram( UINT32 variable, VOID *buffer, UINTN size );
EFI_STATUS VarGetDefaults( UINT32 variable, UINT32 offset, UINTN size, VOID *buffer );
EFI_STATUS VarGetValue( UINT32 variable, UINT32 offset, UINTN size, VOID *buffer );
EFI_STATUS VarSetValue( UINT32 variable, UINT32 offset, UINTN size, VOID *buffer );
VOID VarUpdateVariable(UINT32 variable);
VOID VarUpdateDefaults(UINT32 variable);
VOID *VarGetVariable( UINT32 variable, UINTN *size );

EFI_STATUS _VarGetData( UINT32 variable, UINT32 offset, UINTN size, VOID *buffer, BOOLEAN useDefaults );
EFI_STATUS _VarGetSetValue( UINTN command, NVRAM_VARIABLE *list, UINT32 variable, UINT32 offset, UINTN size, VOID *buffer );

#endif /* _VARIABLE_H_ */

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**         (C)Copyright 2011, American Megatrends, Inc.             **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093       **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
