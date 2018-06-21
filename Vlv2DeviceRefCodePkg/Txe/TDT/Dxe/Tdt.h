/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2004 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Tdt.h

Abstract:

  Defines and prototypes for the TDT driver.
  This driver implements the TDT protocol for Theft Deterrence Technology.

--*/
#ifndef _TDT_H_
#define _TDT_H_

#ifdef ECP_FLAG
#include "SeCLib.h"

//
// Used during initialization
//
#include <Protocol/FirmwareVolume/FirmwareVolume.h>
#include <Protocol/HECI.h>

//
// Driver Produced Protocols
//
#include <Protocol/Tdt.h>
#else
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PerformanceLib.h>
#include <Library/SeCLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

//
// Used during initialization
//
#include <Protocol/FirmwareVolume.h>
#include <Protocol/Heci.h>

//
// Driver Produced Protocols
//
#include <Protocol/Tdt.h>
#endif

//
// extern EFI_GUID gDxePlatformTdtGuid;
//
#define TDT_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('T', 'D', 'T', 'P')

#include "TdtHi.h"

#pragma pack(1)
//
// MKHI host message header. This header is part of HECI message sent from MEBx via
// Host Configuration Interface (HCI). SEC Configuration Manager or Power Configuration
// Manager also include this header with appropriate fields set as part of the
// response message to the HCI.
//
typedef struct {
  UINTN             Signature;
  EFI_HANDLE        Handle;
  EFI_TDT_PROTOCOL  TdtProtocol;

} TDT_INSTANCE;

#define TDT_INSTANCE_FROM_TDT_PROTOCOL(a) CR (a, TDT_INSTANCE, TdtProtocol, TDT_PRIVATE_DATA_SIGNATURE)

#pragma pack()

EFI_STATUS
EFIAPI
AuthenticateCredential (
  IN     EFI_TDT_PROTOCOL   *This,
  IN     UINT8              *PassPhrase,
  IN     UINT32             *PassType,
  IN OUT UINT8              *IsAuthenticated
  )
/*++

Routine Description:
  This API recovers the platform state to Active from Stolen

Arguments:
  This        - The address of protocol
  PassPhrase  - Passphrase that needs to be authenticated sent to SEC
  PassType    - Password type user or server generated
  IsAuthenticated  - The return of the password match 1 for success and 0 for fail

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
GetTdtSeCRule (
  IN     EFI_TDT_PROTOCOL      *This,
  IN OUT UINT8                 *TdtState,
  IN OUT UINT8                 *TdtLastTheftTrigger,
  IN OUT UINT16                *TdtLockState,
  IN OUT UINT16                *TdtAmPref
  )
/*++

Routine Description:

  This function sends a request to SEC Kernel and to find out if TDT is supported and also what is the rule
  data. The rule data defines the state of the TDT Platform. This call also replaces the GetTDTState call for
  BIOS AM module.

Arguments:

  This                - The TDT instance of TDT protocol
  TdtState             - Pointer to AT State Information
  TdtLastTheftTrigger  - Pointer to Variable holding the cause of last AT Stolen Stae
  TdtLockState         - Pointer to variable indicating whether AT is locked or not
  TdtAmPref            - Pointer to variable indicating whether TDTAM or PBA should be used

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
GetNonce (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT8              *Nonce
  )
/*++

Routine Description:
  This gets the SEC nonce

Arguments:
  This        - The address of protocol
  Nonce  -  The return value of the 16 Byte nonce received from SEC

Returns:
  EFI_SUCCESS           The function completed successfully.
--*/
;

EFI_STATUS
EFIAPI
GetTimerInfo (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT32             *Interval,
  IN OUT UINT32             *TimeLeft
  )
/*++

Routine Description:
  This API get the TDT Unlock Timer values

Arguments:
  This        - The address of protocol
  Interval  -  The return value of the Unlock Time Interval that was set by TDT Server
  TimeLeft - The Timeleft in the Unlock Timer

Returns:
  EFI_SUCCESS           The function completed successfully.
--*/
;

EFI_STATUS
EFIAPI
ComputeHash (
  IN     EFI_TDT_PROTOCOL   *This,
  IN     UINT8              *PassPhrase,
  IN OUT UINT8              *Hash
  )
/*++

Routine Description:
  This API compute the SHA1 hash of the user enterted password

Arguments:
  This        - The address of protocol
  PassPhrase  - The passphrase for which SHA1 hash to be computed
  Hash        - The return value of the SHA1 hash

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
GetRecoveryString (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT32                       *StringId,
  IN OUT UINT8                        *IsvString,
  IN OUT UINT32                       *IsvStringLength

  )
/*++

Routine Description:
  This retrives the ISV String stored by TDT Server that BIOS will display during Platform lock state

Arguments:
  This            - The address of protocol
  StringId        - The String buffer ID to retrive the ISV String
  IsvString       - 256 Bytes of ISV string array, the
  IsvStringLength - The String length

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
GetIsvId (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT32                       *IsvId
  )
/*++

Routine Description:
  This receives the ISV ID from SEC and display the ID, when the platform is in stolen state

Arguments:
  This        - The address of protocol
  IsvId       - The pointer to 4 byte ISV ID

Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
SendAssertStolen (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *CompletionCode
  )
/*++

Routine Description:
  This send an AssertStolen Message to SEC when OEM has set the AllowAssertStolen bit to be accepted by BIOS.

Arguments:
  This            - The address of protocol
  CompletionCode  - The return SEC Firmware return code for this request

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
SetSuspendState (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT32                       TransitionState,
  IN     UINT8                        *Token
  )
/*++

Routine Description:
  This Function Processes User requests and FW responses for entering
  and exiting Suspend Mode

Arguments:
  This                - The address of protocol
  TransitionState     - 0: Exit Suspend Mode
                        1: Enter Suspend Mode
  Token               - SRTK generated Token

Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
;

EFI_STATUS
InitWWANREcov (
  IN     EFI_TDT_PROTOCOL             *This
  )
/*++

Routine Description:
  This instructs FW that a WWAN recovery is
  desired and thus the Radio needs to be initialized

Arguments:
  This             - The address of protocol

Returns:
  EFI_SUCCESS       The function completed successfully.

--*/
;

EFI_STATUS
GetWWANNicStatus (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *RadioStatus,
  IN OUT UINT8                        *NetworkStatus
  )
/*++

Routine Description:
  This queries FW of the NIC Radio Status

Arguments:
  This              - The address of protocol
  RadioStatus       - 0: Radio Off
                      1: Radio On
  NetworkStatus     - 0: Detached
                      1: Attached
Returns:
  EFI_SUCCESS         The function completed successfully.

--*/
;

EFI_STATUS
EFIAPI
TdtEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:

  Entry point for the TDTDxe Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
;

#endif
