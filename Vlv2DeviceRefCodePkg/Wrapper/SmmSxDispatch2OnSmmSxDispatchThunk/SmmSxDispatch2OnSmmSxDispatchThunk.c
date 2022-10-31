/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  SMM SxDispatch2 Protocol on SMM SxDispatch Protocol Thunk driver.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <PiDxe.h>
#include <FrameworkSmm.h>

#include <Protocol/SmmSxDispatch2.h>
#include <Protocol/SmmSxDispatch.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

typedef struct {
    LIST_ENTRY                      Link;
    EFI_HANDLE                      DispatchHandle;
    UINTN                           DispatchFunction;
} EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT;


EFI_STATUS
EFIAPI
SmmSxDispatch2Register(
    IN  CONST EFI_SMM_SX_DISPATCH2_PROTOCOL  *This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2   DispatchFunction,
    IN  CONST EFI_SMM_SX_REGISTER_CONTEXT    *RegisterContext,
    OUT       EFI_HANDLE                     *DispatchHandle
);

EFI_STATUS
EFIAPI
SmmSxDispatch2UnRegister(
    IN CONST EFI_SMM_SX_DISPATCH2_PROTOCOL  *This,
    IN       EFI_HANDLE                     DispatchHandle
);

EFI_SMM_SX_DISPATCH2_PROTOCOL gSmmSxDispatch2 = {
    SmmSxDispatch2Register,
    SmmSxDispatch2UnRegister
};

EFI_SMM_SX_DISPATCH_PROTOCOL  *mSmmSxDispatch;
LIST_ENTRY                    mSmmSxDispatch2ThunkQueue = INITIALIZE_LIST_HEAD_VARIABLE(mSmmSxDispatch2ThunkQueue);


/**
  This function find SmmSxDispatch2Context by DispatchHandle.

  @param DispatchHandle The DispatchHandle to identify the SmmSxDispatch2Thunk context

  @return SmmSxDispatch2Thunk context
**/
EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT *
FindSmmSxDispatch2ContextByDispatchHandle(
    IN EFI_HANDLE   DispatchHandle
)
{
    LIST_ENTRY                            *Link;
    EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT    *ThunkContext;

    for(Link = mSmmSxDispatch2ThunkQueue.ForwardLink;
            Link != &mSmmSxDispatch2ThunkQueue;
            Link = Link->ForwardLink) {
        ThunkContext = BASE_CR(
                           Link,
                           EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT,
                           Link
                       );
        if(ThunkContext->DispatchHandle == DispatchHandle) {
            return ThunkContext;
        }
    }
    return NULL;
}

/**
  Framework dispatch function for a Sx SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.

  @return None

**/
VOID
EFIAPI
FrameworkDispatchFunction(
    IN  EFI_HANDLE                    DispatchHandle,
    IN  EFI_SMM_SX_DISPATCH_CONTEXT   *DispatchContext
)
{
    EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
    EFI_SMM_HANDLER_ENTRY_POINT2          DispatchFunction;
    EFI_SMM_SX_REGISTER_CONTEXT           RegisterContext;

    ThunkContext = FindSmmSxDispatch2ContextByDispatchHandle(DispatchHandle);
    ASSERT(ThunkContext != NULL);

    RegisterContext.Type   = DispatchContext->Type;
    RegisterContext.Phase  = DispatchContext->Phase;
    DispatchFunction        = (EFI_SMM_HANDLER_ENTRY_POINT2)ThunkContext->DispatchFunction;
    DispatchFunction(DispatchHandle, &RegisterContext, NULL, 0);
}

/**
  Register a child SMI source dispatch function for the specified software SMI.

  This service registers a function (DispatchFunction) which will be called when the Sx
  SMI source specified by RegisterContext->Type is detected. On return,
  DispatchHandle contains a unique handle which may be used later to unregister the function
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_SX_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction      Function to register for handler when the specified software
                                    SMI is generated.
  @param[in, out]  RegisterContext  Pointer to the dispatch function's context.
                                    The caller fills this context in before calling
                                    the register function to indicate to the register
                                    function which Software SMI input value the
                                    dispatch function should be invoked for.
  @param[out] DispatchHandle        Handle generated by the dispatcher to track the
                                    function instance.

  @retval EFI_SUCCESS               The dispatch function has been successfully
                                    registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR          The Sx driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER     RegisterContext is invalid. The SX SMI input value
                                    is not within valid range.
  @retval EFI_OUT_OF_RESOURCES      There is not enough memory (system or SMM) to manage this
                                    child.
  @retval EFI_OUT_OF_RESOURCES      A unique software SMI value could not be assigned
                                    for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmSxDispatch2Register(
    IN  CONST EFI_SMM_SX_DISPATCH2_PROTOCOL  *This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2   DispatchFunction,
    IN  CONST EFI_SMM_SX_REGISTER_CONTEXT    *RegisterContext,
    OUT       EFI_HANDLE                     *DispatchHandle
)
{
    EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
    EFI_SMM_SX_DISPATCH_CONTEXT           DispatchContext;
    EFI_STATUS                            Status;

    DispatchContext.Type   = RegisterContext->Type;
    DispatchContext.Phase  = RegisterContext->Phase;

    Status = mSmmSxDispatch->Register(
                 mSmmSxDispatch,
                 FrameworkDispatchFunction,
                 &DispatchContext,
                 DispatchHandle
             );

    if(!EFI_ERROR(Status)) {
        Status = gSmst->SmmAllocatePool(
                     EfiRuntimeServicesData,
                     sizeof(*ThunkContext),
                     (VOID **)&ThunkContext
                 );
        ASSERT_EFI_ERROR(Status);
        if(EFI_ERROR(Status)) {
            mSmmSxDispatch->UnRegister(mSmmSxDispatch, *DispatchHandle);
            return EFI_OUT_OF_RESOURCES;
        }

        ThunkContext->DispatchFunction  = (UINTN)DispatchFunction;
        ThunkContext->DispatchHandle    = *DispatchHandle;
        InsertTailList(&mSmmSxDispatch2ThunkQueue, &ThunkContext->Link);
    }

    return Status;
}

/**
  Unregister a child SMI source dispatch function for the specified Sx SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be
  called in response to a Sx SMI.

  @param[in] This                Pointer to the EFI_SMM_SX_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmSxDispatch2UnRegister(
    IN CONST EFI_SMM_SX_DISPATCH2_PROTOCOL  *This,
    IN       EFI_HANDLE                     DispatchHandle
)
{
    EFI_SMM_SX_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
    EFI_STATUS                            Status;

    Status = mSmmSxDispatch->UnRegister(mSmmSxDispatch, DispatchHandle);
    if(!EFI_ERROR(Status)) {
        //
        // Unregister
        //
        ThunkContext = FindSmmSxDispatch2ContextByDispatchHandle(DispatchHandle);
        ASSERT(ThunkContext != NULL);
        if(ThunkContext != NULL) {
            RemoveEntryList(&ThunkContext->Link);
            gSmst->SmmFreePool(ThunkContext);
        }
    }

    return Status;
}

/**
  Entry Point for this thunk driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the EFI System Table.

  @retval EFI_SUCCESS  The entry point is executed successfully.
  @retval other        Some error occurred when executing this entry point.
**/
EFI_STATUS
EFIAPI
SmmSxDispatch2ThunkMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS               Status;

    ///
    /// Locate Framework SMM SxDispatch Protocol
    ///
    Status = gBS->LocateProtocol(&gEfiSmmSxDispatchProtocolGuid, NULL, (VOID **)&mSmmSxDispatch);
    ASSERT_EFI_ERROR(Status);

    ///
    /// Publish PI SMM SxDispatch2 Protocol
    ///
    ImageHandle = NULL;
    Status = gSmst->SmmInstallProtocolInterface(
                 &ImageHandle,
                 &gEfiSmmSxDispatch2ProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &gSmmSxDispatch2
             );
    ASSERT_EFI_ERROR(Status);
    return Status;
}

