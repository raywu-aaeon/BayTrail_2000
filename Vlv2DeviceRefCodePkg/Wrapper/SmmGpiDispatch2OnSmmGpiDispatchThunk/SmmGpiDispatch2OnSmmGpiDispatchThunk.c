/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  SMM GpiDispatch2 Protocol on SMM GpiDispatch Protocol Thunk driver.

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

#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmGpiDispatch.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

#define NUM_SUPPORTED_GPIS  16

typedef struct {
    LIST_ENTRY                    Link;
    EFI_HANDLE                    DispatchHandle;
    UINTN                         DispatchFunction;
} EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT;


EFI_STATUS
EFIAPI
SmmGpiDispatch2Register(
    IN  CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL  *This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2    DispatchFunction,
    IN  CONST EFI_SMM_GPI_REGISTER_CONTEXT    *RegisterContext,
    OUT       EFI_HANDLE                      *DispatchHandle
);

EFI_STATUS
EFIAPI
SmmGpiDispatch2UnRegister(
    IN CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL   *This,
    IN       EFI_HANDLE                       DispatchHandle
);

EFI_SMM_GPI_DISPATCH2_PROTOCOL gSmmGpiDispatch2 = {
    SmmGpiDispatch2Register,
    SmmGpiDispatch2UnRegister,
    NUM_SUPPORTED_GPIS
};

EFI_SMM_GPI_DISPATCH_PROTOCOL *mSmmGpiDispatch;
LIST_ENTRY                    mSmmGpiDispatch2ThunkQueue = INITIALIZE_LIST_HEAD_VARIABLE(mSmmGpiDispatch2ThunkQueue);


/**
  This function find SmmGpiDispatch2Context by DispatchHandle.

  @param DispatchHandle The DispatchHandle to identify the SmmGpiDispatch2Thunk context

  @return SmmGpiDispatch2Thunk context
**/
EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT *
FindSmmGpiDispatch2ContextByDispatchHandle(
    IN EFI_HANDLE   DispatchHandle
)
{
    LIST_ENTRY                            *Link;
    EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT   *ThunkContext;

    for(Link = mSmmGpiDispatch2ThunkQueue.ForwardLink;
            Link != &mSmmGpiDispatch2ThunkQueue;
            Link = Link->ForwardLink) {
        ThunkContext = BASE_CR(
                           Link,
                           EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT,
                           Link
                       );
        if(ThunkContext->DispatchHandle == DispatchHandle) {
            return ThunkContext;
        }
    }
    return NULL;
}

/**
  Framework dispatch function for a Gpi SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.

  @return None

**/
VOID
EFIAPI
FrameworkDispatchFunction(
    IN  EFI_HANDLE                    DispatchHandle,
    IN  EFI_SMM_GPI_DISPATCH_CONTEXT  *DispatchContext
)
{
    EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT   *ThunkContext;
    EFI_SMM_HANDLER_ENTRY_POINT2          DispatchFunction;
    EFI_SMM_GPI_REGISTER_CONTEXT          RegisterContext;
    UINT32                                Index = 1;

    ThunkContext = FindSmmGpiDispatch2ContextByDispatchHandle(DispatchHandle);
    ASSERT(ThunkContext != NULL);

    RegisterContext.GpiNum  = Index << (DispatchContext->GpiNum);
    DispatchFunction        = (EFI_SMM_HANDLER_ENTRY_POINT2)ThunkContext->DispatchFunction;
    DispatchFunction(DispatchHandle, &RegisterContext, NULL, 0);
}

/**
  Register a child SMI source dispatch function for the specified software SMI.

  This service registers a function (DispatchFunction) which will be called when the Gpi
  SMI source specified by RegisterContext->Type is detected. On return,
  DispatchHandle contains a unique handle which may be used later to unregister the function
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_GPI_DISPATCH2_PROTOCOL instance.
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
  @retval EFI_DEVICE_ERROR          The Gpi driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER     RegisterContext is invalid. The Gpi SMI input value
                                    is not within valid range.
  @retval EFI_OUT_OF_RESOURCES      There is not enough memory (system or SMM) to manage this
                                    child.
  @retval EFI_OUT_OF_RESOURCES      A unique software SMI value could not be assigned
                                    for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmGpiDispatch2Register(
    IN  CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL  *This,
    IN        EFI_SMM_HANDLER_ENTRY_POINT2    DispatchFunction,
    IN  CONST EFI_SMM_GPI_REGISTER_CONTEXT    *RegisterContext,
    OUT       EFI_HANDLE                      *DispatchHandle
)
{
    EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT   *ThunkContext;
    EFI_SMM_GPI_DISPATCH_CONTEXT          DispatchContext;
    EFI_STATUS                            Status;
    UINTN                                 Index;
#if defined(EFI64) || defined(EFIx64)
    UINT64                                GpiNum = RegisterContext->GpiNum;
    UINTN                                 MaxBit = 64;
#else
    UINT32                                GpiNum = (UINT32) RegisterContext->GpiNum;
    UINTN                                 MaxBit = 32;
#endif

    for(Index = 0; Index <= MaxBit; Index ++) {
        if(GpiNum == 0) {
            Index --;
            break;
        } else {
            GpiNum >>= 1;
        }
    }

    if(Index >= NUM_SUPPORTED_GPIS) { //EIP149075
        return EFI_INVALID_PARAMETER;
    }

    DispatchContext.GpiNum = Index;
    
    Status = mSmmGpiDispatch->Register(
                 mSmmGpiDispatch,
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
            mSmmGpiDispatch->UnRegister(mSmmGpiDispatch, *DispatchHandle);
            return EFI_OUT_OF_RESOURCES;
        }

        ThunkContext->DispatchFunction  = (UINTN)DispatchFunction;
        ThunkContext->DispatchHandle    = *DispatchHandle;
        InsertTailList(&mSmmGpiDispatch2ThunkQueue, &ThunkContext->Link);
    }

    return Status;
}

/**
  Unregister a child SMI source dispatch function for the specified Gpi SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be
  called in response to a Gpi SMI.

  @param[in] This                Pointer to the EFI_SMM_GPI_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmGpiDispatch2UnRegister(
    IN CONST EFI_SMM_GPI_DISPATCH2_PROTOCOL   *This,
    IN       EFI_HANDLE                       DispatchHandle
)
{
    EFI_SMM_GPI_DISPATCH2_THUNK_CONTEXT   *ThunkContext;
    EFI_STATUS                            Status;

    Status = mSmmGpiDispatch->UnRegister(mSmmGpiDispatch, DispatchHandle);
    if(!EFI_ERROR(Status)) {
        //
        // Unregister
        //
        ThunkContext = FindSmmGpiDispatch2ContextByDispatchHandle(DispatchHandle);
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
SmmGpiDispatch2ThunkMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS               Status;

    ///
    /// Locate Framework SMM GpiDispatch Protocol
    ///
    Status = gBS->LocateProtocol(&gEfiSmmGpiDispatchProtocolGuid, NULL, (VOID **)&mSmmGpiDispatch);
    ASSERT_EFI_ERROR(Status);

    ///
    /// Publish PI SMM GpiDispatch2 Protocol
    ///
    ImageHandle = NULL;
    Status = gSmst->SmmInstallProtocolInterface(
                 &ImageHandle,
                 &gEfiSmmGpiDispatch2ProtocolGuid,
                 EFI_NATIVE_INTERFACE,
                 &gSmmGpiDispatch2
             );
    ASSERT_EFI_ERROR(Status);
    return Status;
}

