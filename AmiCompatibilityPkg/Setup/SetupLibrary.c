//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file
    This file contains the functionality for the "SetupLibrary". These
    functions are to enable the runtime registration of items as opposed
    to the build time.
*/
#include <AmiDxeLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/AmiSetupLibrary.h>
#include <SetupCallbackList.h>

extern DLIST AmiCallbackList;
extern DLIST AmiStringInitializationList;
extern DLIST AmiExtractConfigList;
extern DLIST AmiRouteConfigList;

/**
  Register a callback handler to the callback list. The registered callback will be
  called when a HiiConfigAccess request occurs that matches the QuestionId. 

  @param[in]  QuestionId Id associated with the callback function
  @param[in]  CallbackFunction Callback function to call

  @return EFI_SUCCESS The handler was registered successfully.
  @retval EFI_INVALID_PARAMETER The Callback function was NULL
**/
EFI_STATUS
AmiSetupRegisterCallbackHandler (
    IN EFI_QUESTION_ID              QuestionId,
    IN AMI_SETUP_CALLBACK_FUNCTION  CallbackFunction
  )
{
    EFI_STATUS Status = EFI_INVALID_PARAMETER;

    AMI_CALLBACK_LIST_ENTRY *NewEntry = NULL;

    if(CallbackFunction != NULL)
    {
        // TODO: Verify that the QuestionId doesn't already have an entry?
        Status = gBS->AllocatePool(EfiBootServicesData, sizeof(AMI_CALLBACK_LIST_ENTRY), (VOID**)&NewEntry);
        if(!EFI_ERROR(Status))
        {
            NewEntry->QuestionId = QuestionId;
            NewEntry->Function = CallbackFunction;
            DListAdd(&AmiCallbackList, &(NewEntry->Link));
        }
    }
    return Status;
}

/**
  Unregistered the callback handler that uses the passed CallbackFunction.

  @param[in]  CallbackFunction The callback function to remove from the callback list

  @return EFI_STATUS
  @retval EFI_INVALID_PARAMETER The Callback function was NULL
  @retval EFI_SUCCESS The handler was registered successfully.
  @retval EFI_NOT_FOUND The Handle was invalid
**/
EFI_STATUS
AmiSetupUnRegisterCallbackHandler (
    IN AMI_SETUP_CALLBACK_FUNCTION  CallbackFunction
  )
{
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    AMI_CALLBACK_LIST_ENTRY *NextLink = NULL;
    AMI_CALLBACK_LIST_ENTRY *TempLink = NULL;

    if(CallbackFunction != NULL)
    {
        Status = EFI_NOT_FOUND;
        for(NextLink = (AMI_CALLBACK_LIST_ENTRY*)(AmiCallbackList.pHead);
            (NextLink != NULL) && (TempLink = NextLink, NextLink = (AMI_CALLBACK_LIST_ENTRY*)NextLink->Link.pNext, TRUE) ;
            )
        {
            if(TempLink->Function == CallbackFunction)
            {
                DListDelete(&AmiCallbackList, &(TempLink->Link));
                gBS->FreePool(TempLink);
                Status = EFI_SUCCESS;
            }
        }
    }

    return Status;
}

/**
  Register the String Initialization Function. This function will be called when it is determined that
  the strings in the HiiDatabase need to be updated with their runtime values. This function will called
  registered string initialization function prior to entering setup so that strings referenced in setup
  can be updated with any runtime information.

  @param[in] StringInitFunction Function pointer to the string initialization function

  @return EFI_STATUS
  @retval EFI_INVALID_PARAMETER The StringInitFunction was NULL
  @retval EFI_SUCCESS The StringInitFunction was registered
**/
EFI_STATUS
AmiSetupRegisterStringInitializer (
    AMI_STRING_INIT_FUNCTION StringInitFunction
  )
{
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    AMI_STRING_INITIALIZATION_ENTRY *NewEntry = NULL;

    if(StringInitFunction != NULL)
    {
        Status = gBS->AllocatePool(EfiBootServicesData, sizeof(AMI_STRING_INITIALIZATION_ENTRY), (VOID**)&NewEntry);
        if(!EFI_ERROR(Status))
        {
            NewEntry->Function = StringInitFunction;

            DListAdd(&AmiStringInitializationList, &(NewEntry->Link));
        }
    }
    return Status;
}

/**
  Register into the internal structures the passed function into the list of functions that are called
  through the Hii Access Protocol's ExtractConfig function.
  
  @param[in]  ExtractConfigFunction  Function pointer to the extract config function

  @return EFI_STATUS
  @return EFI_INVALID_PARAMETER The ExtractConfigFunction was NULL
  @retval EFI_SUCCESS  The handler was registered successfully.
**/
EFI_STATUS
AmiSetupRegisterExtractConfig (
    EFI_HII_ACCESS_EXTRACT_CONFIG ExtractConfigFunction
  )
{
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    AMI_EXTRACT_CONFIG_ENTRY *NewEntry = NULL;

    if(ExtractConfigFunction != NULL)
    {
        Status = gBS->AllocatePool(EfiBootServicesData, sizeof(AMI_EXTRACT_CONFIG_ENTRY), (VOID**)&NewEntry);
        if(!EFI_ERROR(Status))
        {
            NewEntry->Function = ExtractConfigFunction;
            DListAdd(&AmiExtractConfigList, &(NewEntry->Link));
        }
    }
    return Status;
}

/**
  Unregister from the internal structures passed function from the list of functions that are called
  through the Hii Access Protocol's ExtractConfig function.
  
  @param[in]  ExtractConfigFunction  Function pointer to the string initialization function

  @retval EFI_SUCCESS The handler was unregistered successfully.
  @retval EFI_INVALID_PARAMETER The FunctionPointer was NULL
  @retval EFI_NOT_FOUND The function did not exist in the list
**/
EFI_STATUS
AmiSetupUnRegisterExtractConfig (
    EFI_HII_ACCESS_EXTRACT_CONFIG ExtractConfigFunction
  )
{
    AMI_EXTRACT_CONFIG_ENTRY *NextLink = NULL;
    AMI_EXTRACT_CONFIG_ENTRY *TempLink = NULL;

    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    if(ExtractConfigFunction != NULL)
    {
        Status = EFI_NOT_FOUND;
        for(NextLink = (AMI_EXTRACT_CONFIG_ENTRY*)(AmiExtractConfigList.pHead);
            (NextLink != NULL) && (TempLink = NextLink, NextLink = (AMI_EXTRACT_CONFIG_ENTRY*)NextLink->Link.pNext, TRUE) ;
            )
        {
            if(TempLink->Function == ExtractConfigFunction)
            {
                DListDelete(&AmiExtractConfigList, &(TempLink->Link));
                gBS->FreePool(TempLink);
                Status = EFI_SUCCESS;
                break;
            }
        }
    }
    return Status;
}

/**
  Register into the internal structures the passed function into the list of functions that are called
  through the Hii Access Protocol's RouteConfig function.
  
  @param[in] AccessRouteConfigFunction Function pointer to the AccessRouteConfig 

  @return EFI_STATUS
  @retval EFI_INVALID_PARAMETER The AccessRouteConfigFunction was NULL
  @retval EFI_SUCCESS The AccessRouteConfigFunction was registered
**/
EFI_STATUS
AmiSetupRegisterAccessRouteConfig (
    EFI_HII_ACCESS_ROUTE_CONFIG AccessRouteConfigFunction
  )
{
    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    AMI_ROUTE_CONFIG_ENTRY *NewEntry = NULL;

    if(AccessRouteConfigFunction != NULL)
    {
        Status = gBS->AllocatePool(EfiBootServicesData, sizeof(AMI_ROUTE_CONFIG_ENTRY), (VOID**)&NewEntry);
        if(!EFI_ERROR(Status))
        {
            NewEntry->Function = AccessRouteConfigFunction;
            DListAdd(&AmiRouteConfigList, &(NewEntry->Link));
        }
    }
    return Status;
}

/**
  Unregister from the internal structures passed function from the list of functions that are called
  through the Hii Access Protocol's RouteConfig function.
  
  @param[in] AccessRouteConfigFunction Function pointer to the AccessRouteConfig 

  @return EFI_STATUS
  @retval EFI_INVALID_PARAMETER The AccessRouteConfigFunction was NULL
  @retval EFI_NOT_FOUND The function was not found
  @retval EFI_SUCCESS The AccessRouteConfigFunction was unregistered
**/
EFI_STATUS
AmiSetupUnRegisterAccessRouteConfig (
    EFI_HII_ACCESS_ROUTE_CONFIG AccessRouteConfigFunction
  )
{
    AMI_ROUTE_CONFIG_ENTRY *NextLink = NULL;
    AMI_ROUTE_CONFIG_ENTRY *TempLink = NULL;

    EFI_STATUS Status = EFI_INVALID_PARAMETER;
    if(AccessRouteConfigFunction != NULL)
    {
        Status = EFI_NOT_FOUND;
        for(NextLink = (AMI_ROUTE_CONFIG_ENTRY*)(AmiRouteConfigList.pHead);
            (NextLink != NULL) && (TempLink = NextLink, NextLink = (AMI_ROUTE_CONFIG_ENTRY*)NextLink->Link.pNext, TRUE) ;
            )
        {
            if(TempLink->Function == AccessRouteConfigFunction)
            {
                DListDelete(&AmiRouteConfigList, &(TempLink->Link));
                gBS->FreePool(TempLink);
                Status = EFI_SUCCESS;
                break;
            }
        }
    }
    return Status;
 }
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
