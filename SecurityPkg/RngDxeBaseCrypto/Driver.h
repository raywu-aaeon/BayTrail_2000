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

#ifndef _RNG_DRIVER_H_
#define _RNG_DRIVER_H_

#include <Uefi.h>

#include <Protocol/DriverBinding.h>
#include <Protocol/ServiceBinding.h>
#include <Protocol/RngProtocol.h>
#include <Protocol/DevicePath.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>

#include "ComponentName.h"

#define RNG_DEVICE_DATA_SIGNATURE  SIGNATURE_32 ('R', 'n', 'g', 'D')

typedef struct {
  UINT32                        Signature;

  EFI_HANDLE                    ControllerHandle;
  EFI_HANDLE                    ImageHandle;

  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;

  //
  // List of RNG_SERVICE_DATA
  //
  LIST_ENTRY                    ServiceList;
  //
  // Number of configured RNG Service Binding child
  //
  UINTN                         ConfiguredChildrenNumber;

} RNG_DEVICE_DATA;

#define RBG_DEVICE_DATA_FROM_THIS(a) \
  CR ( \
  (a), \
  RNG_DEVICE_DATA, \
  RngConfig, \
  RNG_DEVICE_DATA_SIGNATURE \
  )

#define RNG_SERVICE_DATA_SIGNATURE  SIGNATURE_32 ('R', 'n', 'g', 'S')

typedef struct {
  UINT32                        Signature;

  LIST_ENTRY                    Link;

  RNG_DEVICE_DATA               *RngDeviceData;
  EFI_HANDLE                    ServiceHandle;
  EFI_SERVICE_BINDING_PROTOCOL  ServiceBinding;

  LIST_ENTRY                    ChildrenList;
  UINTN                         ChildrenNumber;

} RNG_SERVICE_DATA;

#define RNG_SERVICE_DATA_FROM_THIS(a) \
  CR ( \
  (a), \
  RNG_SERVICE_DATA, \
  ServiceBinding, \
  RNG_SERVICE_DATA_SIGNATURE \
  )

#define RNG_SERVICE_DATA_FROM_LINK(a) \
  CR ( \
  (a), \
  RNG_SERVICE_DATA, \
  Link, \
  RNG_SERVICE_DATA_SIGNATURE \
  )


#define RNG_INSTANCE_DATA_SIGNATURE   SIGNATURE_32 ('R', 'n', 'g', 'I')

#define RNG_INSTANCE_DATA_FROM_THIS(a) \
  CR ( \
  (a), \
  RNG_INSTANCE_DATA, \
  RngProtocol, \
  RNG_INSTANCE_DATA_SIGNATURE \
  )

typedef struct {
  UINT32                          Signature;

  RNG_SERVICE_DATA                *RngServiceData;

  EFI_HANDLE                      Handle;

  LIST_ENTRY                      InstEntry;

  EFI_RNG_PROTOCOL                RngProtocol;

  BOOLEAN                         Destroyed;

} RNG_INSTANCE_DATA;

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
  );

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
  );


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
  );

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
  );

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
  );

//
// Internal function
//

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
  );

/**
  Destroy the RNG device context data.

  @param[in, out]  RngDeviceData      Pointer to the rng device context data.
  @param[in]       ImageHandle        The driver image handle.

**/
VOID
RngDestroyDeviceData (
  IN OUT RNG_DEVICE_DATA   *RngDeviceData,
  IN     EFI_HANDLE        ImageHandle
  );

/**
  Create rng service context data.

  @param[in]       RngDeviceData      Pointer to the rng device context data.

  @return A pointer to RNG_SERVICE_DATA or NULL if failed to create RNG service context.

**/
RNG_SERVICE_DATA *
RngCreateServiceData (
  IN RNG_DEVICE_DATA     *RngDeviceData
  );

/**
  Destroy the RNG service context data.

  @param[in, out]  RngServiceData    Pointer to the rng service context data.

  @retval EFI_SUCCESS           The rng service context is destroyed.
  @retval Others                Errors as indicated.

**/
EFI_STATUS
RngDestroyServiceData (
  IN OUT RNG_SERVICE_DATA    *RngServiceData
  );

/**
  Destroy all child of the RNG service data.

  @param[in, out]  RngServiceData    Pointer to the rng service context data.

  @retval EFI_SUCCESS           All child are destroyed.
  @retval Others                Failed to destroy all child.

**/
EFI_STATUS
RngDestroyServiceChild (
  IN OUT RNG_SERVICE_DATA    *RngServiceData
  );

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
  );

extern EFI_RNG_PROTOCOL mRngProtocol;

#endif
