/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  SMM Periodic Timer Dispatch2 Protocol on SMM Periodic Timer Dispatch
  Protocol Thunk driver.

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

#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmPeriodicTimerDispatch.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>


typedef struct {
    LIST_ENTRY                    Link;
    EFI_HANDLE                    DispatchHandle;
    UINTN                         DispatchFunction;
} EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT;


EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2Register(
    IN  CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL	*This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2    			DispatchFunction,
    IN  CONST EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT		*RegisterContext,
    OUT       EFI_HANDLE								*DispatchHandle
);

EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2UnRegister(
    IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL	*This,
    IN       EFI_HANDLE									DispatchHandle
);

EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2GetNextShorterInterval(
    IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL  *This,
    IN OUT UINT64                                       **SmiTickInterval
);

EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL gSmmPeriodicTimerDispatch2 = {
    SmmPeriodicTimerDispatch2Register,
    SmmPeriodicTimerDispatch2UnRegister,
    SmmPeriodicTimerDispatch2GetNextShorterInterval
};

EFI_SMM_PERIODIC_TIMER_DISPATCH_PROTOCOL	*mSmmPeriodicTimerDispatch;
LIST_ENTRY	mSmmPeriodicTimerDispatch2ThunkQueue = \
        INITIALIZE_LIST_HEAD_VARIABLE(mSmmPeriodicTimerDispatch2ThunkQueue);


/**
  This function find SmmPeriodicTimerDispatch2Context by DispatchHandle.

  @param DispatchHandle The DispatchHandle to identify the SmmPeriodicTimerDispatch2Thunk context

  @return SmmPeriodicTimerDispatch2Thunk context
**/
EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT *
FindSmmPeriodicTimerDispatch2ContextByDispatchHandle(
    IN EFI_HANDLE   DispatchHandle
)
{
    LIST_ENTRY                                      *Link;
    EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT  *ThunkContext;

    for(Link = mSmmPeriodicTimerDispatch2ThunkQueue.ForwardLink;
            Link != &mSmmPeriodicTimerDispatch2ThunkQueue;
            Link = Link->ForwardLink) {
        ThunkContext = BASE_CR(
                           Link,
                           EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT,
                           Link
                       );
        if(ThunkContext->DispatchHandle == DispatchHandle) {
            return ThunkContext;
        }
    }
    return NULL;
}

/**
  Framework dispatch function for a PeriodicTimer SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.

  @return None

**/
VOID
EFIAPI
FrameworkDispatchFunction(
    IN  EFI_HANDLE                              DispatchHandle,
    IN  EFI_SMM_PERIODIC_TIMER_DISPATCH_CONTEXT *DispatchContext
)
{
    EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT  *ThunkContext;
    EFI_SMM_HANDLER_ENTRY_POINT2                    DispatchFunction;
    EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT         RegisterContext;

    ThunkContext = FindSmmPeriodicTimerDispatch2ContextByDispatchHandle(DispatchHandle);
    ASSERT(ThunkContext != NULL);

    RegisterContext.Period          = DispatchContext->Period;
    RegisterContext.SmiTickInterval = DispatchContext->SmiTickInterval;
    DispatchFunction      = (EFI_SMM_HANDLER_ENTRY_POINT2)ThunkContext->DispatchFunction;
    DispatchFunction(DispatchHandle, &RegisterContext, NULL, 0);
}

/**
  Register a child SMI source dispatch function for the specified software SMI.

  This service registers a function (DispatchFunction) which will be called when the PeriodicTimer
  SMI source specified by RegisterContext->Type is detected. On return,
  DispatchHandle contains a unique handle which may be used later to unregister the function
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL instance.
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
  @retval EFI_DEVICE_ERROR          The PeriodicTimer driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER     RegisterContext is invalid. The PeriodicTimer SMI input value
                                    is not within valid range.
  @retval EFI_OUT_OF_RESOURCES      There is not enough memory (system or SMM) to manage this
                                    child.
  @retval EFI_OUT_OF_RESOURCES      A unique software SMI value could not be assigned
                                    for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2Register(
    IN  CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL *This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2              DispatchFunction,
    IN  CONST EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT   *RegisterContext,
    OUT       EFI_HANDLE                                *DispatchHandle
)
{
    EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT  *ThunkContext;
    EFI_SMM_PERIODIC_TIMER_DISPATCH_CONTEXT         DispatchContext;
    EFI_STATUS                                      Status;

    DispatchContext.Period          = RegisterContext->Period;
    DispatchContext.SmiTickInterval = RegisterContext->SmiTickInterval;

    Status = mSmmPeriodicTimerDispatch->Register(
                 mSmmPeriodicTimerDispatch,
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
            mSmmPeriodicTimerDispatch->UnRegister(mSmmPeriodicTimerDispatch, *DispatchHandle);
            return EFI_OUT_OF_RESOURCES;
        }

        ThunkContext->DispatchFunction  = (UINTN)DispatchFunction;
        ThunkContext->DispatchHandle    = *DispatchHandle;
        InsertTailList(&mSmmPeriodicTimerDispatch2ThunkQueue, &ThunkContext->Link);
    }

    return Status;
}

/**
  Unregister a child SMI source dispatch function for the specified PeriodicTimer SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be
  called in response to a PeriodicTimer SMI.

  @param[in] This                Pointer to the EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2UnRegister(
    IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL  *This,
    IN       EFI_HANDLE                                 DispatchHandle
)
{
    EFI_SMM_PERIODIC_TIMER_DISPATCH2_THUNK_CONTEXT  *ThunkContext;
    EFI_STATUS                                      Status;

    Status = mSmmPeriodicTimerDispatch->UnRegister(mSmmPeriodicTimerDispatch, DispatchHandle);
    if(!EFI_ERROR(Status)) {
        //
        // Unregister
        //
        ThunkContext = FindSmmPeriodicTimerDispatch2ContextByDispatchHandle(DispatchHandle);
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
SmmPeriodicTimerDispatch2ThunkMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS               Status;

    ///
    /// Locate Framework SMM PeriodicTimer Dispatch Protocol
    ///
    Status = gBS->LocateProtocol (
                    &gEfiSmmPeriodicTimerDispatchProtocolGuid,
                    NULL,
                    (VOID **)&mSmmPeriodicTimerDispatch
                    );
    ASSERT_EFI_ERROR(Status);

    ///
    /// Publish PI SMM PeriodicTimer Dispatch2 Protocol
    ///
    ImageHandle = NULL;
    Status = gSmst->SmmInstallProtocolInterface (
                      &ImageHandle,
                      &gEfiSmmPeriodicTimerDispatch2ProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      &gSmmPeriodicTimerDispatch2
                      );
    ASSERT_EFI_ERROR(Status);
    return Status;
}

EFI_STATUS
EFIAPI
SmmPeriodicTimerDispatch2GetNextShorterInterval(
    IN CONST EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL  *This,
    IN OUT UINT64                                       **SmiTickInterval
)
{
    EFI_STATUS                                      Status;

    Status = mSmmPeriodicTimerDispatch->GetNextShorterInterval(
                 mSmmPeriodicTimerDispatch,
                 SmiTickInterval
                 );

    return Status;
}
