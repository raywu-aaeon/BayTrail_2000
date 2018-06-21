/** @file
  Implement the I2C host protocol.

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cHost.h"


/**
  Handle the I2C bus configuration available event

  This routine is called at TPL_I2C_SYNC.

  @param[in] Event    Address of an EFI_EVENT handle
  @param[in] Context  Address of an I2C_HOST_CONTEXT structure

**/
VOID
EFIAPI
I2cHostI2cBusConfigurationAvailable (
  IN EFI_EVENT Event_not_used,
  IN VOID *Context
  )
{
  I2C_HOST_CONTEXT *I2cHost;
  CONST EFI_I2C_MASTER_PROTOCOL * I2cMaster;
  I2C_REQUEST *I2cRequest;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostI2cBusConfigurationAvailable entered\r\n" ));

  //
  //  Mark this I2C bus configuration management operation as complete
  //
  I2cHost = (I2C_HOST_CONTEXT *)Context;
  I2cHost->I2cBusConfigurationManagementPending = FALSE;

  //
  //  Determine if a request is pending
  //
  I2cRequest = I2cHost->RequestListHead;
  if ( NULL != I2cRequest ) {
    //
    //  Display the request state
    //
    if ( I2cHost->I2cBusConfiguration != I2cRequest->I2cBusConfiguration ) {
      DEBUG (( DEBUG_I2C_OPERATION,
               "0x%016Lx: I2cHost, I2C bus configuration %d for request packet 0x%016Lx, Status: %r\r\n",
               (UINT64)(UINTN)I2cHost,
               I2cRequest->I2cBusConfiguration,
               (UINT64)(UINTN)&I2cRequest->RequestPacket,
               I2cHost->Status ));
    }

    //
    //  Determine if the driver is shutting down
    //
    if ( I2cHost->ShuttingDown ) {
      //
      //  Abort this request
      //
      I2cHostRequestCompleteError ( I2cHost, EFI_ABORTED  );
    } else {
      //
      //  Validate the completion status
      //
      if ( EFI_ERROR ( I2cHost->Status )) {
        I2cHostRequestCompleteError ( I2cHost, I2cHost->Status );

        //
        //  Unknown I2C bus configuration
        //  Force next operation to enable the I2C bus configuration
        //
        I2cHost->I2cBusConfiguration = (UINTN)-1;
      } else {
        //
        //  Update the I2C bus configuration
        //
        I2cHost->I2cBusConfiguration = I2cRequest->I2cBusConfiguration;

        //
        //  Clear the event
        //
        gBS->CheckEvent ( I2cHost->I2cEvent );

        //
        //  Display the request state
        //
        DEBUG (( DEBUG_I2C_OPERATION,
                 "0x%016Lx: I2cHost starting I2C for request packet 0x%016Lx,Event:0x%016Lx\r\n",
                 (UINT64)(UINTN)I2cHost,
                 (UINT64)(UINTN)&I2cRequest->RequestPacket,
                 (UINT64)(UINTN)&I2cHost->I2cEvent));

        //
        //  Start an I2C operation on the host
        //
        I2cMaster = I2cHost->I2cMaster;
        I2cMaster->StartRequest ( I2cMaster,
                                  I2cRequest->SlaveAddress,
                                  I2cHost->I2cEvent,
                                  &I2cRequest->RequestPacket,
                                  &I2cHost->Status );
        DEBUG (( DEBUG_I2C_OPERATION,
                 "0x%016Lx: I2cHost ending I2C for request packet , I2cHost->I2cEvent:0x%016Lx\r\n",
                 (UINT64)(UINTN)I2cHost,
                 (UINT64)(UINTN)&I2cHost->I2cEvent));
      }
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostI2cBusConfigurationAvailable exiting\r\n" ));
}


/**
  Complete the current request

  This routine is called at TPL_I2C_SYNC.

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I2C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestComplete (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  )
{
  I2C_REQUEST *I2cRequest;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestComplete entered\r\n" ));

  //
  //  Remove the current I2C request from the list
  //
  I2cRequest = I2cHost->RequestListHead;
  I2cHost->RequestListHead = I2cRequest->Next;
  if ( NULL == I2cHost->RequestListHead ) {
    I2cHost->RequestListTail = NULL;
  }

  //
  //  Display the request state
  //
  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: I2cHost removed I2C request packet 0x%016Lx from queue\r\n",
           (UINT64)(UINTN)I2cHost,
           (UINT64)(UINTN)&I2cRequest->RequestPacket ));

  //
  //  Display the request state
  //
  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: I2cHost, I2C request packet 0x%016Lx completion, Status: %r\r\n",
           (UINT64)(UINTN)I2cHost,
           (UINT64)(UINTN)&I2cRequest->RequestPacket,
           Status ));

  //
  //  Save the status for the user
  //
  if ( NULL != I2cRequest->Status ) {
    *I2cRequest->Status = Status;
  }

  //
  //  Notify the user of the I2C request completion
  //
  if ( NULL != I2cRequest->Event ) {
    DEBUG((DEBUG_I2C_OPERATION,"Signalling I2CRequest: 0x%016Lx I2cRequest->Event 0x%016Lx\r\n",
           (UINT64)(UINTN)I2cRequest,
           (UINT64)(UINTN)I2cRequest->Event));

    gBS->SignalEvent ( I2cRequest->Event );
  }

  //
  //  Done with this request
  //
  FreePool ( I2cRequest );
  if( I2cHost->RequestListHead ) {
    DEBUG((DEBUG_I2C_OPERATION, "Handle the request in the queue\r\n"));
    I2cHostRequestEnable(I2cHost);
  } else {
    DEBUG((DEBUG_I2C_OPERATION, "This is the last request in the queue\r\n"));
  }


  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestComplete exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Complete the current request with an error

  @param[in] I2cHost  Address of an I2C_HOST_CONTEXT structure.
  @param[in] Status   Status of the I<sub>2</sub>C operation.

  @return This routine returns the input status value.

**/
EFI_STATUS
I2cHostRequestCompleteError (
  I2C_HOST_CONTEXT *I2cHost,
  EFI_STATUS Status
  )
{
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteError entered\r\n" ));

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

  //
  //  Complete the request
  //
  I2cHostRequestComplete ( I2cHost, Status );

  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL ( TplPrevious );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteError exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Handle the bus available event

  This routine is called at TPL_I2C_SYNC.

  @param[in] Event    Address of an EFI_EVENT handle
  @param[in] Context  Address of an I2C_HOST_CONTEXT structure

**/
VOID
EFIAPI
I2cHostRequestCompleteEvent (
  IN EFI_EVENT Event,
  IN VOID *Context
  )
{
  I2C_HOST_CONTEXT *I2cHost;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteEvent entered\r\n" ));

  //
  //  Handle the completion event
  //
  I2cHost = (I2C_HOST_CONTEXT *)Context;
  I2cHostRequestComplete ( I2cHost, I2cHost->Status );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestCompleteEvent exiting\r\n" ));
}


/**
  Enable access to the I2C bus configuration

  @param[in] I2cHost    Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         I2C slave address exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
I2cHostRequestEnable (
  I2C_HOST_CONTEXT *I2cHost
  )
{
  UINTN I2cBusConfiguration;
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;
  I2C_REQUEST *I2cRequest;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestEnable entered\r\n" ));

  //
  //  Assume pending request
  //
  Status = EFI_NOT_READY;

  //
  //  Validate the I2c bus configuration
  //
  I2cRequest = I2cHost->RequestListHead;
  if ( I2cHost->I2cBusConfigurationCount <= I2cRequest->I2cBusConfiguration ) {
    DEBUG (( DEBUG_WARN,
             "WARNING - I2cBusConfiguration too large!\r\n" ));
    Status = I2cHostRequestCompleteError ( I2cHost, EFI_NO_MAPPING );
  } else {
    //
    //  The I2C bus configuration is valid
    //  Determine if the I2C request requires access to another
    //  I2C bus configuration
    //
    I2cBusConfiguration = I2cRequest->I2cBusConfiguration;
    if ( I2cHost->I2cBusConfiguration != I2cBusConfiguration ) {
      //
      //  Display the request state
      //
      DEBUG (( DEBUG_I2C_OPERATION,
               "0x%016Lx: I2cHost configuring access from %d to I2C bus configuration %d for request packet 0x%016Lx\r\n",
               (UINT64)(UINTN)I2cHost,
               I2cHost->I2cBusConfiguration,
               I2cRequest->I2cBusConfiguration,
               (UINT64)(UINTN)&I2cRequest->RequestPacket ));

      //
      //  This case only occurs when I2cBus is non-NULL!
      //
      I2cBusConfigurationManagement = I2cHost->I2cBusConfigurationManagement;

      //
      //  Clear the event
      //
      gBS->CheckEvent ( I2cHost->I2cBusConfigurationEvent );

      //
      //  Another I2C bus configuration is required for this request
      //  Enable access to the required I2C bus configuration
      //
      I2cHost->I2cBusConfigurationManagementPending = TRUE;
      Status = I2cBusConfigurationManagement->EnableI2cBusConfiguration (
                 I2cBusConfigurationManagement,
                 I2cBusConfiguration,
                 I2cHost->I2cMaster,
                 I2cHost->I2cBusConfigurationEvent,
                 &I2cHost->Status );

      //
      //  N.B. This routine is not allowed to access the
      //  I2cRequest after this point!  It is possible for the
      //  port driver to complete this request at any time including
      //  immediately.
      //
      DEBUG((DEBUG_I2C_OPERATION,
             "INFO - Platfrom EnableI2cBusConfiguration  Status: %r\r\n",
             Status ));

      if ( EFI_SUCCESS == Status ) {
        //
        //  Display the request state
        //
        DEBUG (( DEBUG_I2C_OPERATION,
                 "INFO - Platform driver should be returning EFI_NOT_READY, Status: %r\r\n",
                 Status ));

        //
        //  The platform code should always return not ready!
        //  The I2cHostI2cBusConfigurationAvailable will continue to
        //  drive the state machine to complete the I2C request.
        //
        Status = EFI_NOT_READY;
      }
    } else {
      //
      //  Synchronize with the other threads
      //
      DEBUG((DEBUG_I2C_OPERATION, "Same bus configuration\r\n", TPL_I2C_SYNC));
      DEBUG((DEBUG_I2C_OPERATION, "!!!RaiseTPL to %d...\r\n", TPL_I2C_SYNC));
      TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );
      DEBUG((DEBUG_I2C_OPERATION, "!!!RaiseTPL to %d,raised, previous Tpl %d\r\n", TPL_I2C_SYNC, TplPrevious));

      //
      //  Same I2C bus configuration
      //
      I2cHost->Status = EFI_SUCCESS;
      I2cHostI2cBusConfigurationAvailable ( I2cHost->I2cBusConfigurationEvent,
                                            I2cHost );

      //
      //  Release the thread synchronization
      //
      DEBUG((DEBUG_I2C_OPERATION, "!!!Restore TPL to %d...\r\n", TplPrevious));
      gBS->RestoreTPL ( TplPrevious );
      DEBUG((DEBUG_I2C_OPERATION, "!!!Restore TPL to %d,restored\r\n", TplPrevious));
    }
  }

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestEnable exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Queue an I2C operation for execution on the I2C controller.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumers of this API are the I2C bus driver and
  on rare occasions the I2C test application.  Extreme care must be
  taken by other consumers of this API to prevent confusing the
  third party I2C drivers due to a state change at the I2C device
  which the third party I2C drivers did not initiate.  I2C platform
  drivers may use this API within these guidelines.

  This layer uses the concept of I2C bus configurations to describe
  the I2C bus.  An I2C bus configuration is defined as a unique
  setting of the multiplexers and switches in the I2C bus which
  enable access to one or more I2C devices.  When using a switch
  to divide a bus, due to speed differences, the I2C platform layer
  would define an I2C bus configuration for the I2C devices on each
  side of the switch.  When using a multiplexer, the I2C platform
  layer defines an I2C bus configuration for each of the selector
  values required to control the multiplexer.  See Figure 1 in the
  <a href="http://www.nxp.com/documents/user_manual/UM10204.pdf">I<sup>2</sup>C
  Specification</a> for a complex I2C bus configuration.

  The I2C host driver processes all operations in FIFO order.  Prior to
  performing the operation, the I2C host driver calls the I2C platform
  driver to reconfigure the switches and multiplexers in the I2C bus
  enabling access to the specified I2C device.  The I2C platform driver
  also selects the maximum bus speed for the device.  After the I2C bus
  is configured, the I2C host driver calls the I2C port driver to
  initialize the I2C controller and start the I2C operation.

  @param[in] This             Address of an EFI_I2C_HOST_PROTOCOL instance.
  @param[in] I2cBusConfiguration  I2C bus configuration to access the I2C
                                  device.
  @param[in] SlaveAddress     Address of the device on the I2C bus.
  @param[in] Event            Event to set for asynchronous operations,
                              NULL for synchronous operations
  @param[in] RequestPacket    Address of an EFI_I2C_REQUEST_PACKET
                              structure describing the I2C operation
  @param[out] I2cStatus       Optional buffer to receive the I2C operation
                              completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         I2C slave address exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cHostRequestQueue (
  IN CONST EFI_I2C_HOST_PROTOCOL *This,
  IN UINTN I2cBusConfiguration,
  IN UINTN SlaveAddress,
  IN EFI_EVENT Event OPTIONAL,
  IN CONST EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  )
{
  I2C_HOST_CONTEXT *I2cHost;
  I2C_REQUEST *I2cRequest;
  I2C_REQUEST *Previous;
  BOOLEAN StartRequest;
  EFI_STATUS Status;
  EFI_EVENT SyncEvent;
  EFI_TPL TplPrevious;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestQueue entered\r\n" ));
  //
  //  Allocate the event if necessary
  //
  SyncEvent = NULL;
  Status = EFI_SUCCESS;
  if ( NULL == Event ) {
    Status = gBS->CreateEvent ( 0,
                                TPL_I2C_SYNC,
                                NULL,
                                NULL,
                                &SyncEvent );
    DEBUG((DEBUG_I2C_OPERATION, "CreateEvent for syn operation, SyncEvent: 0x%016Lx\r\n",(UINT64)(UINTN)SyncEvent));
  }
  if ( !EFI_ERROR ( Status )) {
    //
    //  Assume pending request
    //
    Status = EFI_NOT_READY;

    //
    //  Validate the request packet
    //
    if ( NULL == RequestPacket ) {
      DEBUG (( DEBUG_WARN,
               "WARNING - RequestPacket is NULL\r\n" ));
      Status = EFI_INVALID_PARAMETER;
    } else {
      //
      //  Validate the TPL
      //
      TplPrevious = gBS->RaiseTPL ( TPL_HIGH_LEVEL );
      gBS->RestoreTPL ( TplPrevious );
      DEBUG((DEBUG_I2C_OPERATION,"!!!Current Tpl: %d\r\n",TplPrevious));
      if (( TPL_I2C_SYNC < TplPrevious )
          || (( NULL == Event ) && ( TPL_CALLBACK < TplPrevious ))) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - TPL %d is too high!\r\n",
                 TplPrevious  ));
        Status = EFI_INVALID_PARAMETER;
      } else {
        //
        //  Allocate the request structure
        //
        I2cHost = I2C_HOST_CONTEXT_FROM_PROTOCOL ( This );
        I2cRequest = AllocateZeroPool ( sizeof ( *I2cRequest ));
        if ( NULL == I2cRequest ) {
          DEBUG (( DEBUG_WARN,
                   "WARNING - Failed to allocate I2C_REQUEST!\r\n" ));
          Status = EFI_OUT_OF_RESOURCES;
        } else {
          //
          //  Initialize the request
          //
          I2cRequest->I2cBusConfiguration = I2cBusConfiguration;
          I2cRequest->SlaveAddress = SlaveAddress;
          I2cRequest->Event = ( NULL == Event ) ? SyncEvent : Event;
          I2cRequest->Status = I2cStatus;
          CopyMem ( &I2cRequest->RequestPacket, RequestPacket, sizeof ( *RequestPacket ));

          //
          //  Display the request state
          //
          DEBUG (( DEBUG_I2C_OPERATION,
                   "0x%016Lx: I2cHost queuing I2C request 0x%016Lx packet 0x%016Lx\r\n",
                   (UINT64)(UINTN)I2cHost,
                   (UINT64)(UINTN)I2cRequest,
                   (UINT64)(UINTN)&I2cRequest->RequestPacket ));

          //
          //  Synchronize with the other threads
          //
          DEBUG((DEBUG_I2C_OPERATION,"!!!RaiseTPL to %d...\r\n",TPL_I2C_SYNC));
          gBS->RaiseTPL ( TPL_I2C_SYNC );
          DEBUG((DEBUG_I2C_OPERATION,"!!!RaiseTPL to %d,raised\r\n",TPL_I2C_SYNC));

          //
          //  Place the request at the end of the pending list
          //
          Previous = I2cHost->RequestListTail;
          StartRequest = (BOOLEAN)( NULL == Previous );
          if ( !StartRequest ) {
            //
            //  Another request is pending
            //  Place this request at the end of the list
            //
            Previous->Next = I2cRequest;
          } else {
            //
            //  This is the first request
            //
            I2cHost->RequestListHead = I2cRequest;
          }
          I2cHost->RequestListTail = I2cRequest;

          //
          //  Release the thread synchronization
          //
          DEBUG((DEBUG_I2C_OPERATION,"!!!Restore TPL to %d...\r\n",TplPrevious));
          gBS->RestoreTPL ( TplPrevious );
          DEBUG((DEBUG_I2C_OPERATION,"!!!Restore TPL to %d,restored\r\n",TplPrevious));

          //
          //  N.B. This routine is not allowed to access the
          //  I2cRequest after this point!  It is possible for the
          //  port driver to complete this request at any time including
          //  immediately.
          //
          //  Start processing this request
          //
          if ( StartRequest ) {
            //
            //  Enable access to the I2C bus configuration
            //
            Status = I2cHostRequestEnable ( I2cHost );
          }

          //
          //  Check for a synchronous operation
          //
          DEBUG((DEBUG_I2C_OPERATION, "Status: %r\r\n", Status));
          if (( NULL == Event )
              && ( EFI_NOT_READY == Status )) {
            //
            //  Display the request state
            //
            DEBUG (( DEBUG_I2C_OPERATION,
                     "0x%016Lx: I2cHost waiting for synchronous I2C request packet 0x%016Lx\r\n",
                     (UINT64)(UINTN)I2cHost,
                     (UINT64)(UINTN)RequestPacket ));

            //
            //  Wait for the operation completion
            //
            do {
              DEBUG (( DEBUG_I2C_OPERATION,
                       "Check SyncEvent I2cRequest->Event 0x%016Lx...\r\n",
                       (UINT64)(UINTN)SyncEvent));
              Status = gBS->CheckEvent ( SyncEvent );
            } while ( EFI_NOT_READY == Status );

            //
            //  Get the operation status
            //
            Status = I2cHost->Status;

            //
            //  Display the request state
            //
            DEBUG (( DEBUG_I2C_OPERATION,
                     "0x%016Lx: I2cHost, synchronous I2C request packet 0x%016Lx complete, Status: %r\r\n",
                     (UINT64)(UINTN)I2cHost,
                     (UINT64)(UINTN)RequestPacket,
                     Status ));

            //
            //  Return the operation status
            //
            if ( NULL != I2cStatus ) {
              *I2cStatus = Status;
            }
          }
        }
      }
    }
  }

  //
  //  Close the event if necessary
  //
  if ( NULL != SyncEvent ) {
    gBS->CloseEvent ( SyncEvent );
  }

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostRequestQueue exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cHostDriverStart to complete the driver
  initialization.

  @param [in] I2cHost         Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized

**/
EFI_STATUS
I2cHostApiStart (
  IN I2C_HOST_CONTEXT *I2cHost
  )
{
  CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *I2cBusConfigurationManagement;
  CONST EFI_I2C_MASTER_PROTOCOL *I2cMaster;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStart entered\r\n" ));

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Reset the controller
  //
  I2cMaster = I2cHost->I2cMaster;
  I2cMaster->Reset ( I2cMaster );

  //
  //  Get the I2C event
  //
  Status = gBS->CreateEvent ( EVT_NOTIFY_SIGNAL,
                              TPL_I2C_SYNC,
                              &I2cHostRequestCompleteEvent,
                              I2cHost,
                              &I2cHost->I2cEvent );
  if ( EFI_ERROR ( Status )) {
    DEBUG (( DEBUG_ERROR,
             "ERROR - Failed to allocate the I2C event, Status: %r\r\n",
             Status ));
  } else {
    //
    //  Determine the number of I2C bus configurations
    //
    I2cHost->I2cBusConfigurationCount = 1;
    I2cBusConfigurationManagement = I2cHost->I2cBusConfigurationManagement;
    if ( NULL != I2cBusConfigurationManagement ) {
      I2cHost->I2cBusConfigurationCount = I2cBusConfigurationManagement->I2cBusConfigurationCount;

      //
      //  Get the bus management event
      //
      Status = gBS->CreateEvent ( EVT_NOTIFY_SIGNAL,
                                  TPL_I2C_SYNC,
                                  I2cHostI2cBusConfigurationAvailable,
                                  I2cHost,
                                  &I2cHost->I2cBusConfigurationEvent );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Enable the primary I2C bus configuration
        //
        DEBUG((EFI_D_INFO, "I2cBusConfiguration: %d, set it to -1\n", I2cHost->I2cBusConfiguration));
        I2cHost->I2cBusConfiguration = (UINTN)-1;
        DEBUG((EFI_D_INFO, "Do we need to set the primary bus configuration here?\n"));
        /*Status = I2cBusConfigurationManagement->EnableI2cBusConfiguration (
                          I2cBusConfigurationManagement,
                          I2cHost->I2cBusConfiguration,
                          I2cHost->I2cMaster,
                          NULL,
                          NULL );
                          */
      }
    }
  }

  //
  //  Build the I2C host protocol
  //
  I2cHost->HostApi.I2cBusConfigurationCount = I2cHost->I2cBusConfigurationCount;
  I2cHost->HostApi.QueueRequest = I2cHostRequestQueue;
  I2cHost->HostApi.MaximumReceiveBytes = I2cMaster->MaximumReceiveBytes;
  I2cHost->HostApi.MaximumTransmitBytes = I2cMaster->MaximumTransmitBytes;
  I2cHost->HostApi.MaximumTotalBytes = I2cMaster->MaximumTotalBytes;

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the startup status
  //
  return Status;
}


/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cHostDriverStop to initiate the driver
  shutdown.

  @param [in] I2cHost         Address of an I2C_HOST_CONTEXT structure

**/
VOID
I2cHostApiStop (
  IN I2C_HOST_CONTEXT *I2cHost
  )
{
  I2C_REQUEST *I2cRequest;
  I2C_REQUEST *I2cRequestList;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStop entered\r\n" ));

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

  //
  //  Flag the driver shutdown
  //
  I2cHost->ShuttingDown = TRUE;

  //
  //  Remove all but the active request from the pending list
  //
  I2cRequestList = I2cHost->RequestListHead;
  if (( NULL != I2cRequestList )
      && ( !I2cHost->I2cBusConfigurationManagementPending )) {
    I2cRequestList = I2cRequestList->Next;
    I2cHost->RequestListHead->Next = NULL;
    I2cHost->RequestListTail = I2cHost->RequestListHead;
  }

  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL ( TplPrevious );

  //
  //  Abort any pending requests
  //
  while ( NULL != I2cRequestList ) {
    I2cRequest = I2cRequestList;
    I2cRequestList = I2cRequest->Next;

    //
    //  Abort this request
    //
    if ( NULL != I2cRequest->Status ) {
      *I2cRequest->Status = EFI_ABORTED;
    }
    if ( NULL != I2cRequest->Event ) {
      gBS->SignalEvent ( I2cRequest->Event );
    }

    //
    //  Done with this request
    //
    FreePool ( I2cRequest );
  }

  //
  //  Wait for the bus management to complete
  //
  while ( I2cHost->I2cBusConfigurationManagementPending ) {
  };

  //
  //  Release the events
  //
  if ( NULL != I2cHost->I2cBusConfigurationEvent ) {
    Status = gBS->CloseEvent ( I2cHost->I2cBusConfigurationEvent );
    ASSERT ( EFI_SUCCESS == Status );
    I2cHost->I2cBusConfigurationEvent = NULL;
  }

  if ( NULL != I2cHost->I2cEvent ) {
    Status = gBS->CloseEvent ( I2cHost->I2cEvent );
    ASSERT ( EFI_SUCCESS == Status );
    I2cHost->I2cEvent = NULL;
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cHostApiStop exiting\r\n" ));
}
