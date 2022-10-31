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

EFI_DRIVER_BINDING_PROTOCOL gRngDriverBinding = {
  RngDriverBindingSupported,
  RngDriverBindingStart,
  RngDriverBindingStop,
  0xa,
  NULL,
  NULL
};

BOOLEAN mInitialized = FALSE;

/**
  Test to see if this driver supports ControllerHandle. This service
  is called by the EFI boot service ConnectController(). In
  order to make drivers as small as possible, there are a few calling
  restrictions for this service. ConnectController() must
  follow these calling restrictions. If any other agent wishes to call
  Supported() it must also follow these calling restrictions.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  ControllerHandle     Handle of device to test.
  @param[in]  RemainingDevicePath  Optional parameter use to pick a specific
                                   child device to start.

  @retval EFI_SUCCESS              This driver supports this device.
  @retval EFI_ALREADY_STARTED      This driver is already running on this device.
  @retval Others                   This driver does not support this device.

**/
EFI_STATUS
EFIAPI
RngDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS                   Status;
  EFI_DEVICE_PATH_PROTOCOL     *DevicePath;

  if (mInitialized) {
    return EFI_ALREADY_STARTED;
  }

  //
  // Test to open the Device Path protocol BY_DRIVER.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EFI_UNSUPPORTED;
  if ((DevicePath->Type == ACPI_DEVICE_PATH) && (DevicePath->SubType == ACPI_DP)) {
    if (((ACPI_HID_DEVICE_PATH *)DevicePath)->HID == EISA_PNP_ID (0x0C31)) {
      Status = EFI_SUCCESS;
    }
  }

  //
  // Close the openned Device Path protocol.
  //
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiDevicePathProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
         );

  return Status;
}


/**
  Start this driver on ControllerHandle. This service is called by the
  EFI boot service ConnectController(). In order to make drivers as small
  as possible, there are a few calling restrictions for this service.
  ConnectController() must follow these calling restrictions. If any other
  agent wishes to call Start() it must also follow these calling restrictions.

  @param[in]       This                 Protocol instance pointer.
  @param[in]       ControllerHandle     Handle of device to bind driver to.
  @param[in]       RemainingDevicePath  Optional parameter use to pick a specific
                                        child device to start.

  @retval EFI_SUCCESS           This driver is added to ControllerHandle.
  @retval EFI_ALREADY_STARTED   This driver is already running on ControllerHandle.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory for Rng Service Data.
  @retval Others                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
RngDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS        Status;
  RNG_SERVICE_DATA  *RngServiceData;
  RNG_DEVICE_DATA   *RngDeviceData;
  LIST_ENTRY        *Entry;

  //
  // Initialize the Rng Device Data
  //
  DEBUG((EFI_D_ERROR, "RngDriverBindingStart \n"));
  RngDeviceData = AllocateZeroPool (sizeof (RNG_DEVICE_DATA));
  if (RngDeviceData == NULL) {
    DEBUG ((EFI_D_ERROR, "RngDriverBindingStart(): Failed to allocate the Rng Device Data.\n"));

    return EFI_OUT_OF_RESOURCES;
  }

  Status = RngInitializeDeviceData (RngDeviceData, This->DriverBindingHandle, ControllerHandle);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "RngDriverBindingStart: RngInitializeDeviceData failed, %r.\n", Status));

    FreePool (RngDeviceData);
	DEBUG((EFI_D_ERROR, "RngDriverBindingStart 1 %r\n", Status));
    return Status;
  }

  //
  // Check whether driver has already produced protocol
  //

  //
  // create a default RNG service data for untagged frame
  //
  RngServiceData = RngCreateServiceData (RngDeviceData);
  Status = (RngServiceData != NULL) ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
  goto Exit;

Exit:

  if (EFI_ERROR (Status)) {
    //
    // Destroy all RNG service data
    //
    while (!IsListEmpty (&RngDeviceData->ServiceList)) {
      Entry = GetFirstNode (&RngDeviceData->ServiceList);
      RngServiceData = RNG_SERVICE_DATA_FROM_LINK (Entry);
      RngDestroyServiceData (RngServiceData);
    }

    //
    // Destroy Rng Device Data
    //
    RngDestroyDeviceData (RngDeviceData, This->DriverBindingHandle);
    FreePool (RngDeviceData);
  }

  if (Status == EFI_SUCCESS) {
    mInitialized = TRUE;
  }

  DEBUG((EFI_D_ERROR, "RngDriverBindingStart 2 %r\n", Status));
  return Status;
}

/**
  Stop this driver on ControllerHandle. This service is called by the
  EFI boot service DisconnectController(). In order to make drivers as
  small as possible, there are a few calling restrictions for this service.
  DisconnectController() must follow these calling restrictions. If any other
  agent wishes to call Stop() it must also follow these calling restrictions.

  @param[in]  This               Protocol instance pointer.
  @param[in]  ControllerHandle   Handle of device to stop driver on.
  @param[in]  NumberOfChildren   Number of Handles in ChildHandleBuffer. If
                                 number of children is zero stop the entire
                                 bus driver.
  @param[in]  ChildHandleBuffer  List of Child Handles to Stop.

  @retval EFI_SUCCESS            This driver is removed ControllerHandle.
  @retval EFI_DEVICE_ERROR       The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
RngDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      ControllerHandle,
  IN UINTN                           NumberOfChildren,
  IN EFI_HANDLE                      *ChildHandleBuffer OPTIONAL
  )
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *ServiceBinding;
  RNG_DEVICE_DATA               *RngDeviceData;
  RNG_SERVICE_DATA              *RngServiceData;
  BOOLEAN                       AllChildrenStopped;
  LIST_ENTRY                    *Entry;

  //
  // Try to retrieve RNG service binding protocol from the ControllerHandle
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiRngServiceBindingProtocolGuid,
                  (VOID **) &ServiceBinding,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "RngDriverBindingStop: try to stop unknown Controller.\n"));
    return EFI_DEVICE_ERROR;
  } else {
    RngServiceData = RNG_SERVICE_DATA_FROM_THIS (ServiceBinding);
    RngDeviceData = RngServiceData->RngDeviceData;
  }

  if (NumberOfChildren == 0) {
    //
    // Destroy all RNG service data
    //
    while (!IsListEmpty (&RngDeviceData->ServiceList)) {
      Entry = GetFirstNode (&RngDeviceData->ServiceList);
      RngServiceData = RNG_SERVICE_DATA_FROM_LINK (Entry);
      RngDestroyServiceData (RngServiceData);
    }

    //
    // Destroy Rng Device Data
    //
    RngDestroyDeviceData (RngDeviceData, This->DriverBindingHandle);
    FreePool (RngDeviceData);

    mInitialized = FALSE;

    return EFI_SUCCESS;
  }

  //
  // Stop all RNG child
  //
  AllChildrenStopped = TRUE;
  for(Entry = (&RngDeviceData->ServiceList)->ForwardLink; Entry != (&RngDeviceData->ServiceList); Entry = Entry->ForwardLink) {
    RngServiceData = RNG_SERVICE_DATA_FROM_LINK (Entry);

    Status = RngDestroyServiceChild (RngServiceData);
    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Creates a child handle with a set of I/O services.

  @param[in]       This              Protocol instance pointer.
  @param[in, out]  ChildHandle       Pointer to the handle of the child to create. If
                                     it is NULL, then a new handle is created. If
                                     it is not NULL, then the I/O services are added
                                     to the existing child handle.

  @retval EFI_SUCCES                 The protocol was added to ChildHandle.
  @retval EFI_INVALID_PARAMETER      ChildHandle is NULL.
  @retval EFI_OUT_OF_RESOURCES       There are not enough resources availabe to
                                     create the child.
  @retval Others                     The child handle was not created.

**/
EFI_STATUS
EFIAPI
RngServiceBindingCreateChild (
  IN     EFI_SERVICE_BINDING_PROTOCOL    *This,
  IN OUT EFI_HANDLE                      *ChildHandle
  )
{
  EFI_STATUS         Status;
  RNG_SERVICE_DATA   *RngServiceData;
  RNG_INSTANCE_DATA  *Instance;
  VOID               *RngSb;
  EFI_TPL            OldTpl;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  RngServiceData = RNG_SERVICE_DATA_FROM_THIS (This);

  //
  // Allocate buffer for the new instance.
  //
  Instance = AllocateZeroPool (sizeof (RNG_INSTANCE_DATA));
  if (Instance == NULL) {
    DEBUG ((EFI_D_ERROR, "RngServiceBindingCreateChild: Faild to allocate memory for the new instance.\n"));

    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Init the instance data.
  //
  RngInitializeInstanceData (RngServiceData, Instance);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiRngProtocolGuid,
                  &Instance->RngProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG (
      (EFI_D_ERROR,
      "RngServiceBindingCreateChild: Failed to install the RNG protocol, %r.\n",
      Status)
      );

    goto ErrorExit;
  }

  //
  // Save the instance's childhandle.
  //
  Instance->Handle = *ChildHandle;

  Status = gBS->OpenProtocol (
                  RngServiceData->ServiceHandle,
                  &gEfiRngServiceBindingProtocolGuid,
                  (VOID **) &RngSb,
                  gRngDriverBinding.DriverBindingHandle,
                  Instance->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  //
  // Add the child instance into ChildrenList.
  //
  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  InsertTailList (&RngServiceData->ChildrenList, &Instance->InstEntry);
  RngServiceData->ChildrenNumber++;

  gBS->RestoreTPL (OldTpl);

ErrorExit:

  if (EFI_ERROR (Status)) {

    if (Instance->Handle != NULL) {

      gBS->UninstallMultipleProtocolInterfaces (
            Instance->Handle,
            &gEfiRngProtocolGuid,
            &Instance->RngProtocol,
            NULL
            );
    }

    FreePool (Instance);
  }

  return Status;
}


/**
  Destroys a child handle with a set of I/O services.

  The DestroyChild() function does the opposite of CreateChild(). It removes a
  protocol that was installed by CreateChild() from ChildHandle. If the removed
  protocol is the last protocol on ChildHandle, then ChildHandle is destroyed.

  @param[in]  This               Pointer to the EFI_SERVICE_BINDING_PROTOCOL
                                 instance.
  @param[in]  ChildHandle        Handle of the child to destroy.

  @retval EFI_SUCCES             The protocol was removed from ChildHandle.
  @retval EFI_UNSUPPORTED        ChildHandle does not support the protocol that
                                 is being removed.
  @retval EFI_INVALID_PARAMETER  ChildHandle is NULL.
  @retval EFI_ACCESS_DENIED      The protocol could not be removed from the
                                 ChildHandle because its services are being
                                 used.
  @retval Others                 The child handle was not destroyed.

**/
EFI_STATUS
EFIAPI
RngServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                      ChildHandle
  )
{
  EFI_STATUS                    Status;
  RNG_SERVICE_DATA              *RngServiceData;
  EFI_RNG_PROTOCOL              *RngProtocol;
  RNG_INSTANCE_DATA             *Instance;
  EFI_TPL                       OldTpl;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  RngServiceData = RNG_SERVICE_DATA_FROM_THIS (This);

  //
  // Try to retrieve Rng Protocol from ChildHandle.
  //
  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiRngProtocolGuid,
                  (VOID **) &RngProtocol,
                  gRngDriverBinding.DriverBindingHandle,
                  ChildHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Instance = RNG_INSTANCE_DATA_FROM_THIS (RngProtocol);

  //
  // RngServiceBindingDestroyChild may be called twice: first called by
  // RngServiceBindingStop, second called by uninstalling the RNG protocol
  // in this ChildHandle. Use destroyed to make sure the resource clean code
  // will only excecute once.
  //
  if (Instance->Destroyed) {
    return EFI_SUCCESS;
  }

  Instance->Destroyed = TRUE;

  //
  // Close the Device Path protocol.
  //
  gBS->CloseProtocol (
         RngServiceData->ServiceHandle,
         &gEfiRngServiceBindingProtocolGuid,
         RngServiceData->RngDeviceData->ImageHandle,
         ChildHandle
         );

  //
  // Uninstall the Rng protocol.
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiRngProtocolGuid,
                  &Instance->RngProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    DEBUG (
      (EFI_D_ERROR,
      "RngServiceBindingDestroyChild: Failed to uninstall the Rng protocol, %r.\n",
      Status)
      );

    Instance->Destroyed = FALSE;
    return Status;
  }

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  //
  // Remove this instance from the ChildrenList.
  //
  RemoveEntryList (&Instance->InstEntry);
  RngServiceData->ChildrenNumber--;

  gBS->RestoreTPL (OldTpl);

  FreePool (Instance);

  return Status;
}

/**
  The entry point for Rng driver which installs the driver binding and component
  name protocol on its ImageHandle.

  @param[in]  ImageHandle  The image handle of the driver.
  @param[in]  SystemTable  The system table.

  @retval EFI_SUCCES       The driver binding and component name protocols are
                           successfully installed.
  @retval Others           Other errors as indicated.

**/
EFI_STATUS
EFIAPI
RngDriverEntryPoint (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  )
{
  return EfiLibInstallDriverBindingComponentName2 (
           ImageHandle,
           SystemTable,
           &gRngDriverBinding,
           ImageHandle,
           &gRngComponentName,
           &gRngComponentName2
           );
}
