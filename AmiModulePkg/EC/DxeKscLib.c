/*++

Copyright (c)  1999 - 2005 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DxeKscLib.c

Abstract:

  Boot service DXE KSC library implementation.

  These functions in this file can be called during DXE

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/

#include "Library/KscLib.h"
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>

BOOLEAN mDxeKscLibInitialized = FALSE;

//
// Function implemenations
//
EFI_STATUS
InitializeKscLib (
  VOID
  )
/*++

Routine Description:

  Initialize the library.
  Read KSC Command port (0x66), if found 0xff means no EC exists else EC exists

Arguments:

  None.

Returns:

  EFI_SUCCESS       - KscLib is successfully initialized.

--*/
{
  //
  // Fail if EC doesn't exist.
  //
  if(IoRead8(KSC_C_PORT) == 0xff){
    mDxeKscLibInitialized = FALSE;
    return EFI_DEVICE_ERROR;
  }

  mDxeKscLibInitialized = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
SendKscCommand (
  UINT8   Command
  )
/*++

Routine Description:

  Sends command to Keyboard System Controller.

Arguments:

  Command  - Command byte to send

Returns:

  EFI_SUCCESS       - Command success
  EFI_DEVICE_ERROR  - Command error

--*/
{
  UINTN   Index;
  UINT8   KscStatus = 0;

  //
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE) {
    return EFI_DEVICE_ERROR;
  }

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus);
  while (((KscStatus & KSC_S_IBF) != 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send the KSC command
  //
  IoWrite8(KSC_C_PORT, Command);

  return EFI_SUCCESS;
}

EFI_STATUS
ReceiveKscStatus (
  UINT8   *KscStatus
  )
/*++

Routine Description:

  Receives status from Keyboard System Controller.

Arguments:

  KscStatus  - Status byte to receive

Returns:

  EFI_SUCCESS       - Always success

--*/
{
  //
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Read and return the status
  //
  *KscStatus = IoRead8(KSC_C_PORT);

  return EFI_SUCCESS;
}

EFI_STATUS
SendKscData (
  UINT8   Data
  )
/*++

Routine Description:

  Sends data to Keyboard System Controller.

Arguments:

  Data  - Data byte to send

Returns:

  EFI_SUCCESS       - Success
  EFI_DEVICE_ERROR  - Error

--*/
{
  UINTN   Index;
  UINT8   KscStatus;

  //
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE) {
    return EFI_DEVICE_ERROR;
  }

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus);
  while (((KscStatus & KSC_S_IBF) != 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Send the data and return
  //
  IoWrite8(KSC_D_PORT, Data);

  return EFI_SUCCESS;
}

EFI_STATUS
ReceiveKscData (
  UINT8   *Data
  )
/*++

Routine Description:

  Receives data from Keyboard System Controller.

Arguments:

  Data  - Data byte received

Returns:

  EFI_SUCCESS       - Read success
  EFI_DEVICE_ERROR  - Read error

--*/
{
  UINTN   Index;
  UINT8   KscStatus;

  //
  // Verify if KscLib has been initialized, NOT if EC dose not exist.
  //
  if(mDxeKscLibInitialized == FALSE) {
    return EFI_DEVICE_ERROR;
  }

  Index = 0;

  //
  // Wait for KSC to be ready (with a timeout)
  //
  ReceiveKscStatus (&KscStatus);
  while (((KscStatus & KSC_S_OBF) == 0) && (Index < KSC_TIME_OUT)) {
    gBS->Stall (15);
    ReceiveKscStatus (&KscStatus);
    Index++;
  }
  if (Index >= KSC_TIME_OUT) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Read KSC data and return
  //
  *Data = IoRead8(KSC_D_PORT);

  return EFI_SUCCESS;
}
