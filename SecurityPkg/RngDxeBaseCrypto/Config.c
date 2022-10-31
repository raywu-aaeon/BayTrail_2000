/** @file

  Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
  This software and associated documentation
  (if any) is furnished under a license and may only be used or
  copied in accordance with the terms of the license. Except as
  permitted by such license, no part of this software or
  documentation may be reproduced, stored in a retrieval system, or
  transmitted in any form or by any means without the express
  written consent of Intel Corporation.

**/

#include "Driver.h"

EFI_SERVICE_BINDING_PROTOCOL    mRngServiceBindingProtocol = {
  RngServiceBindingCreateChild,
  RngServiceBindingDestroyChild
};

/**
  Initialize the rng device context data.

  @param[in, out]  RngDeviceData      Pointer to the rng device context data.
  @param[in]       ImageHandle        The driver image handle.
  @param[in]       ControllerHandle   Handle of device to bind driver to.

  @retval EFI_SUCCESS           The rng service context is initialized.
  @retval EFI_UNSUPPORTED       ControllerHandle does not support Device Path Protocol.
  @retval Others                Other errors as indicated.

**/
EFI_STATUS
RngInitializeDeviceData (
  IN OUT RNG_DEVICE_DATA   *RngDeviceData,
  IN     EFI_HANDLE        ImageHandle,
  IN     EFI_HANDLE        ControllerHandle
  )
{
  EFI_STATUS                  Status;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  RngDeviceData->Signature        = RNG_DEVICE_DATA_SIGNATURE;
  RngDeviceData->ImageHandle      = ImageHandle;
  RngDeviceData->ControllerHandle = ControllerHandle;

  //
  // Open the DevicePath protocol.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  ImageHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  RngDeviceData->DevicePath = DevicePath;

  //
  // Initialize the lists.
  //
  InitializeListHead (&RngDeviceData->ServiceList);

  if (EFI_ERROR (Status)) {
    //
    // Close the DevicePath Protocol.
    //
    gBS->CloseProtocol (
          ControllerHandle,
          &gEfiDevicePathProtocolGuid,
          ImageHandle,
          ControllerHandle
          );
  }

  return Status;
}


/**
  Destroy the RNG device context data.

  @param[in, out]  RngDeviceData      Pointer to the rng device context data.
  @param[in]       ImageHandle        The driver image handle.

**/
VOID
RngDestroyDeviceData (
  IN OUT RNG_DEVICE_DATA   *RngDeviceData,
  IN     EFI_HANDLE        ImageHandle
  )
{
  //
  // Close the DevicePath Protocol.
  //
  gBS->CloseProtocol (
         RngDeviceData->ControllerHandle,
         &gEfiDevicePathProtocolGuid,
         ImageHandle,
         RngDeviceData->ControllerHandle
         );
}


/**
  Create rng service context data.

  @param[in]       RngDeviceData      Pointer to the rng device context data.

  @return A pointer to RNG_SERVICE_DATA or NULL if failed to create RNG service context.

**/
RNG_SERVICE_DATA *
RngCreateServiceData (
  IN RNG_DEVICE_DATA     *RngDeviceData
  )
{
  EFI_HANDLE                RngServiceHandle;
  RNG_SERVICE_DATA          *RngServiceData;
  EFI_STATUS                Status;

  //
  // Initialize the Rng Service Data.
  //
  RngServiceData = AllocateZeroPool (sizeof (RNG_SERVICE_DATA));
  if (RngServiceData == NULL) {
    DEBUG ((EFI_D_ERROR, "RngCreateServiceData: Faild to allocate memory for the new Rng Service Data.\n"));

    return NULL;
  }

  //
  // Add to RNG service list
  //
  InsertTailList (&RngDeviceData->ServiceList, &RngServiceData->Link);

  RngServiceData->Signature     = RNG_SERVICE_DATA_SIGNATURE;
  RngServiceData->RngDeviceData = RngDeviceData;

  //
  // Copy the ServiceBinding structure.
  //
  CopyMem (&RngServiceData->ServiceBinding, &mRngServiceBindingProtocol, sizeof (EFI_SERVICE_BINDING_PROTOCOL));

  //
  // Initialize the lists.
  //
  InitializeListHead (&RngServiceData->ChildrenList);

  RngServiceHandle    = RngDeviceData->ControllerHandle;

  RngServiceData->ServiceHandle = RngServiceHandle;

  //
  // Install the RNG Service Binding Protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &RngServiceHandle,
                  &gEfiRngServiceBindingProtocolGuid,
                  &RngServiceData->ServiceBinding,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    RngDestroyServiceData (RngServiceData);
    RngServiceData = NULL;
  }

  return RngServiceData;
}

/**
  Destroy the RNG service context data.

  @param[in, out]  RngServiceData    Pointer to the rng service context data.

  @retval EFI_SUCCESS           The rng service context is destroyed.
  @retval Others                Errors as indicated.

**/
EFI_STATUS
RngDestroyServiceData (
  IN OUT RNG_SERVICE_DATA    *RngServiceData
  )
{
  EFI_STATUS  Status;

  //
  // Uninstall the RNG Service Binding Protocol
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  RngServiceData->ServiceHandle,
                  &gEfiRngServiceBindingProtocolGuid,
                  &RngServiceData->ServiceBinding,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Remove from RngDeviceData service list
  //
  RemoveEntryList (&RngServiceData->Link);

  FreePool (RngServiceData);

  return Status;
}

/**
  Destroy all child of the RNG service data.

  @param[in, out]  RngServiceData    Pointer to the rng service context data.

  @retval EFI_SUCCESS           All child are destroyed.
  @retval Others                Failed to destroy all child.

**/
EFI_STATUS
RngDestroyServiceChild (
  IN OUT RNG_SERVICE_DATA    *RngServiceData
  )
{
  EFI_STATUS                    Status;
  RNG_INSTANCE_DATA             *Instance;
  EFI_SERVICE_BINDING_PROTOCOL  *ServiceBinding;

  ServiceBinding = &RngServiceData->ServiceBinding;
  while (!IsListEmpty (&RngServiceData->ChildrenList)) {
    //
    // Don't use ListRemoveHead here, the remove opreration will be done
    // in ServiceBindingDestroyChild.
    //
    Instance = BASE_CR (
                 (&RngServiceData->ChildrenList)->ForwardLink,
                 RNG_INSTANCE_DATA,
                 InstEntry
                 );

    Status = ServiceBinding->DestroyChild (ServiceBinding, Instance->Handle);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Initialize the rng instance context data.

  @param[in]       RngServiceData   Pointer to the rng service context data.
  @param[in, out]  Instance         Pointer to the rng instance context data
                                    to initialize.

**/
VOID
RngInitializeInstanceData (
  IN     RNG_SERVICE_DATA    *RngServiceData,
  IN OUT RNG_INSTANCE_DATA   *Instance
  )
{
  //
  // Set the signature.
  //
  Instance->Signature = RNG_INSTANCE_DATA_SIGNATURE;

  //
  // Copy the RNG Protocol interfaces from the template.
  //
  CopyMem (&Instance->RngProtocol, &mRngProtocol, sizeof (Instance->RngProtocol));

  //
  // Save the RngServiceData info.
  //
  Instance->RngServiceData = RngServiceData;
}

