//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/componentname.h 9     5/03/11 10:49a Ryanchou $
//
// $Revision: 9 $
//
// $Date: 5/03/11 10:49a $
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           ComponentName.h
//
//  Description:    AMI USB driver component name header file
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __COMPONENTNAME_H__
#define __COMPONENTNAME_H__

//#include <Protocol/ComponentName.h>
typedef CHAR16* (*COMPONENT_CB_T)(EFI_HANDLE controller,EFI_HANDLE child);

typedef struct {
    EFI_COMPONENT_NAME2_PROTOCOL icn;
    CHAR16                      *driver_name;
    CHAR16                      *component_static;
    COMPONENT_CB_T              component_cb;
} NAME_SERVICE_T;
                                        //(EIP59272)>
EFI_COMPONENT_NAME2_PROTOCOL*
InitNamesStatic( NAME_SERVICE_T* n , CHAR16* driver, CHAR16* component );

EFI_COMPONENT_NAME2_PROTOCOL*
InitNamesProtocol( NAME_SERVICE_T* n , CHAR16* driver, COMPONENT_CB_T component_cb);

//
// EFI Component Name Functions
//
EFI_STATUS
AMIUSBComponentNameGetDriverName (
  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  CHAR8                        *Language,
  CHAR16                       **DriverName
  );

EFI_STATUS
AMIUSBComponentNameGetControllerName (
  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  EFI_HANDLE                   ControllerHandle,
  EFI_HANDLE                   ChildHandle        OPTIONAL,
  CHAR8                        *Language,
  CHAR16                       **ControllerName
  );
                                        //<(EIP59272)
#endif //__COMPONENTNAME_H__

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
