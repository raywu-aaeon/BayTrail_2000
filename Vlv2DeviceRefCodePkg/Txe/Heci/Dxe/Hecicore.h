/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciCore.h

Abstract:

  Definitions for HECI driver

--*/
#ifndef _HECI_CORE_H
#define _HECI_CORE_H

#include "Hecidrv.h"
#include "HeciRegs.h"
#include "HeciHpet.h"
#include <CoreBiosMsg.h>
#include <SeCState.h>
#ifdef ECP_FLAG
#include "SeCLib.h"
#else
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Library/SeCLib.h>
#endif

//
// HECI bus function version
//
#define HBM_MINOR_VERSION 0
#define HBM_MAJOR_VERSION 1

//
// Prototypes
//
EFI_STATUS
EFIAPI
HeciInitialize (
  VOID
  )
/*++

  Routine Description:
    Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciReInitialize (
  VOID
  )
/*++

  Routine Description:
    Heci Re-initializes it for Host

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciReInitialize2 (
  VOID
  )
/*++

  Routine Description:
    Heci Re-initializes it for SeC

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciReceive (
  IN      UINT32                    Blocking,
  OUT     UINT32                    *MessageData,
  IN OUT  UINT32                    *Length
  )
/*++

  Routine Description:
    Reads a message from the ME across HECI.

  Arguments:
    Blocking    - Used to determine if the read is BLOCKING or NON_BLOCKING.
    MessageData - Pointer to a buffer used to receive a message.
    Length      - Pointer to the length of the buffer on input and the length
                  of the message on return. (in bytes)

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciSend (
  IN      UINT32                    *Message,
  IN      UINT32                    Length,
  IN      UINT8                     HostAddress,
  IN      UINT8                     SeCAddress
  )
/*++

  Routine Description:
    Function sends one messsage (of any length) through the HECI circular buffer.

  Arguments:
    Message     - Pointer to the message data to be sent.
    Length      - Length of the message in bytes.
    HostAddress - The address of the host processor.
    SeCAddress   - Address of the ME subsystem the message is being sent to.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciSendwACK (
  IN OUT  UINT32                    *Message,
  IN      UINT32                    Length,
  IN OUT  UINT32                    *RecLength,
  IN      UINT8                     HostAddress,
  IN      UINT8                     SeCAddress
  )
/*++

  Routine Description:
    Function sends one messsage through the HECI circular buffer and waits
    for the corresponding ACK message.

  Arguments:
    Message     - Pointer to the message buffer.
    SendLength  - Length of the message in bytes.
    RecLength   - Length of the message response in bytes.
    HostAddress - Address of the sending entity.
    SeCAddress   - Address of the ME entity that should receive the message.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
SeCResetWait (
  IN  UINT32  Delay
  )
/*++

Routine Description:

  SeC reset and waiting for ready

Arguments:

  Delay - The biggest waiting time

Returns:

  EFI_TIMEOUT - Time out
  EFI_SUCCESS - SeC ready

--*/
;

EFI_STATUS
EFIAPI
ResetHeciInterface (
  VOID
  )
/*++

  Routine Description:
    Function forces a reinit of the heci interface by following the reset heci interface via host algorithm
    in HPS 0.90 doc 4-17-06 njy

  Arguments:
    none

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciGetSeCStatus (
  IN UINT32                     *SeCStatus
  )
/*++

  Routine Description:
    Return ME Status

  Arguments:
    SeCStatus pointer for status report

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciGetSeCMode (
  IN UINT32                     *SeCMode
  )
/*++

  Routine Description:
    Return ME Mode

  Arguments:
    SeCMode pointer for ME Mode report

  Returns:
    EFI_STATUS

--*/
;

//
// Local/Private functions not part of EFIAPI for HECI
//
EFI_STATUS
InitializeHECI (
  IN      EFI_HANDLE                ImageHandle,
  IN      EFI_SYSTEM_TABLE          *SystemTable
  )
/*++

Routine Description:
  HECI driver entry point used to initialize support for the HECI device.

Arguments:
  ImageHandle - Standard entry point parameter.
  SystemTable - Standard entry point parameter.

Returns:
  EFI_STATUS

--*/
;

EFI_STATUS
InitializeHeciPrivate (
  VOID
  )
/*++

  Routine Description:
    Determines if the HECI device is present and, if present, initializes it for
    use by the BIOS.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
;

UINT32
CheckAndFixHeciForAccess (
  VOID
  )
/*++

Routine Description:
  This function provides a standard way to verify the HECI cmd and MBAR regs
  in its PCI cfg space are setup properly and that the local mHeciContext
  variable matches this info.

Arguments:
  None.

Returns:
  VOID

--*/
;

EFI_STATUS
WaitForSECInputReady (
  VOID
  )
/*++

  Routine Description:
    Waits for the ME to report that it is ready for communication over the HECI
    interface.

  Arguments:
    None.

  Returns:
    EFI_STATUS

--*/
;

UINT8
FilledSlots (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer
  )
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer  - Location of the read pointer.
    WritePointer - Location of the write pointer.

  Returns:
    Number of filled slots.

--*/
;

EFI_STATUS
OverflowCB (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer,
  IN      UINT32                    BufferDepth
  )
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer - Value read from host/me read pointer
    WritePointer - Value read from host/me write pointer
    BufferDepth - Value read from buffer depth register

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
HeciPacketRead (
  IN      UINT32                    Blocking,
  OUT     HECI_MESSAGE_HEADER       *MessageHeader,
  OUT     UINT32                    *MessageData,
  IN OUT  UINT32                    *Length
  )
/*++

  Routine Description:
    Function to pull one messsage packet off the HECI circular buffer.
    Corresponds to HECI HPS (part of) section 4.2.4


  Arguments:
    Blocking      - Used to determine if the read is BLOCKING or NON_BLOCKING.
    MessageHeader - Pointer to a buffer for the message header.
    MessageData   - Pointer to a buffer to recieve the message in.
    Length        - On input is the size of the callers buffer in bytes.  On
                    output this is the size of the packet in bytes.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
HeciPacketWrite (
  IN      HECI_MESSAGE_HEADER       *MessageHeader,
  IN      UINT32                    *MessageData
  )
/*++

  Routine Description:
   Function sends one messsage packet through the HECI buffer
   Corresponds to HECI HPS (part of) section 4.2.3

  Arguments:
    MessageHeader - Pointer to the message header.
    MessageData   - Pointer to the actual message data.

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
SeCAlivenessRequest (
  IN      UINT32                       *HeciMemBar,
  IN      UINT32                       Request
  );


#endif // _HECI_CORE_H
