/** @file
Provides library services of Flea Market Interface.
Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/



#ifndef _FMI_LIB_H
#define _FMI_LIB_H
#include <Uefi/UefiBaseType.h>
#include <Base.h>


/**
  This function initialises FMI state machine. It should be called before calling any FMI interface
  @param[in]  None
  @return Return EFI_SUCCESS if FMI is in READY state and CMD/RESPONSE buffer is allocated successfully.
**/
EFI_STATUS
EFIAPI
FmiInit (
  VOID
  );


/**
  This function authenticates IBB's(initial boot block) integrity.
  @param[in]  IBBBuffer     IBB memory buffer
  @param[in]  IBBSize       size of IBB
  @return Return EFI_SUCCESS if IBB is verified successfully
**/
EFI_STATUS
EFIAPI
FmiAuthIBB (
  IN UINT8 *IBBBuffer,
  IN UINT32 IBBSize
  );


/**
  This command will verify an RSA 2048 signature verification using SHA256 \
  as the hash following PKCS1 v1.5 schema or PKCS1 v2.2 (PSS) schema.
  VLV will support currently only PKCS1 v1.5 schema.
  Assuming a public exponent 17d or 65537d, \
  the time of signature verification may be between 12 ms to 15 ms and \
  therefore we will support only those public exponents.
**/
EFI_STATUS
EFIAPI
FmiRSAVerify(
  IN UINT32 pkcs,
  IN UINT8 *modulus,
  IN UINT32 public_e,
  IN UINT8 *signature,
  IN UINT8 *digest
  );


/**
  This command disables the ability to enable the debug capabilities.
**/
EFI_STATUS
EFIAPI
FmiDebugCtrlDisable(
  VOID
  );


/**
  This command enables/disables the OEM debug capabilities (EXI and Prob Mode)
  After this command is sent, BIOS MUST induce a global reset to enable the debug capabilities.
**/
EFI_STATUS
EFIAPI
FmiSetDebugState(
  IN UINT32 state
  );


/**
  This function must be called to free resources allocated.
  @param[in]  None
  @return Return EFI_SUCCESS if resources is freed successfully
**/
EFI_STATUS
EFIAPI
FreeFmiResource(
  VOID
  );


#endif
