//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/debug.c 7     5/16/08 12:01p Olegi $
//
// $Revision: 7 $
//
// $Date: 5/16/08 12:01p $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           Debug.c
//
//  Description:    AMI USB Debug output implementation routnes
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        PrintDebugMessage (variable param)
//
// Description: This routine prints the debug message
//
// Parameters: Variable
//
// Output:      Status: SUCCESS = Success
//                      FAILURE = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

RETCODE
PrintDebugMsg(
    int MsgLevel,
    char * Message, ...)
{
	va_list ArgList;

    va_start(ArgList, Message);

    if ((MsgLevel == 0) ||
        ((MsgLevel <= TopDebugLevel) &&
            (MsgLevel >= BottomDebugLevel)))
    {
#if USB_DEBUG_MESSAGES == 1
        EfiDebugVPrint(EFI_D_ERROR, Message, ArgList);
#endif
    }

    va_end(ArgList);

    return SUCCESS;
}
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
