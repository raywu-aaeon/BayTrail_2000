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

#include <Uefi/UefiBaseType.h>
#include <Uefi/UefiSpec.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/FmiLib.h>
#include "Fmi.h"
EFI_PHYSICAL_ADDRESS CmdBuffer = (EFI_PHYSICAL_ADDRESS)(UINTN) NULL;
UINT32     NrPages = 0;
EFI_PHYSICAL_ADDRESS ReallocIBBBuffer = (EFI_PHYSICAL_ADDRESS)(UINTN) NULL;
UINT32     IBBPages = 0;

static
EFI_STATUS
WaitFlipChange(
  IN UINT32 PreviousSlip
  )
{
  UINT32 TryCount = 5000;
  EFI_STATUS Status;
  DEBUG((EFI_D_INFO, "WaitFlipChange Entry\r\n"));
  DEBUG((EFI_D_INFO, "Original FLIP: 0x%08x\r\n", PreviousSlip));
  while(TryCount --) {
    gBS->Stall(2000);
    if(FMI_GET_FLIP != PreviousSlip) {
      DEBUG((EFI_D_INFO, "Changed Status register: 0x%08x\r\n", FMI_GetRawStatus));
      break;
    }
  }
  Status = TryCount > 0 ? EFI_SUCCESS : EFI_TIMEOUT;
  DEBUG((EFI_D_INFO, "WaitFlipChange Exit with Status: %r\r\n", Status));
  return Status;
}

static
EFI_STATUS
EFIAPI
IssueFmiInit (
  VOID
  )
{
  UINT32 State;
  EFI_STATUS Status;
  UINT32  PreviousSlip;
  DEBUG((EFI_D_INFO, "IssueFmiInit Entry\r\n"));
  PreviousSlip = FMI_GET_FLIP;
  FMI_WRITE_ICR(0);
  Status = WaitFlipChange(PreviousSlip);
  if(!EFI_ERROR(Status)) {
    State = FMI_GET_STATE;
    if(State != FMI_STATUS_STATE_READY) {
      Status = EFI_DEVICE_ERROR;
    }
  }
  DEBUG((EFI_D_INFO, "IssueFmiInit Exit with Status: %r\r\n", Status));
  return Status;
}

#if 0
static
EFI_STATUS
IssueFmiShutdown (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}
#endif

EFI_STATUS
EFIAPI
FmiInit (
  VOID
  )
{
  UINT32 State = 0;
  UINT32 BufferSize = 0;
  EFI_STATUS Status;
  DEBUG((EFI_D_INFO, "FmiInit Entry\r\n"));
  State = FMI_GET_STATE;
  if(State == FMI_STATUS_STATE_CLOSE) {
    Status = EFI_DEVICE_ERROR;
    goto _exitFmiInit;
  }
  /*FMI buffer size doesn't always reflect buffer size, it is usually zero in the middle of FMI command*/
  if( (State != FMI_STATUS_STATE_READY) ||
      (FMI_GET_BUFFER_SIZE != 1)
    ) {
    Status = IssueFmiInit();
    if(EFI_ERROR(Status)) {
      goto _exitFmiInit;
    }
  }
  BufferSize = FMI_GET_BUFFER_SIZE;
  if( (BufferSize == 0) ||
      (BufferSize > 256) ) {
    Status = EFI_DEVICE_ERROR;
    goto _exitFmiInit;
  }
  NrPages = EFI_SIZE_TO_PAGES (BufferSize * 0x400);

  CmdBuffer = (EFI_PHYSICAL_ADDRESS) 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                AllocateMaxAddress,
                EfiBootServicesData,
                (UINTN)NrPages,
                &CmdBuffer
                );
  DEBUG((EFI_D_INFO, "FmiInit allocated %d pages of buffer at 0x%08x\r\n", NrPages, (UINT32)CmdBuffer));
  if(EFI_ERROR(Status)) {
    CmdBuffer = (EFI_PHYSICAL_ADDRESS)(UINTN) NULL;
  }
_exitFmiInit:
  DEBUG((EFI_D_INFO, "FmiInit Exit with Status: %r\r\n", Status));
  return Status;
}

static
EFI_STATUS
EFIAPI
IssueFmiCmd (
  VOID
  )
{
  UINT32 State;
  UINT32 PreviousSlip;
  EFI_STATUS Status;
  if(CmdBuffer == (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    Status = EFI_DEVICE_ERROR;
    goto _exit_IssueFmiCmd;
  }
  State = FMI_GET_STATE;
  if(State != FMI_STATUS_STATE_READY) {
    Status = IssueFmiInit();
    if(EFI_ERROR(Status)) {
      Status = EFI_DEVICE_ERROR;
      goto _exit_IssueFmiCmd;
    }
  }
  PreviousSlip = FMI_GET_FLIP;
  FMI_ENABLE_SATT23;
  FMI_WRITE_ICR((UINT32)CmdBuffer);
  Status = WaitFlipChange(PreviousSlip);
  FMI_DISABLE_SATT23;
  if(EFI_ERROR(Status)) {
    Status = EFI_DEVICE_ERROR;
    goto _exit_IssueFmiCmd;
  }
  State = FMI_GET_STATE;
  Status = (State == FMI_STATUS_STATE_READY) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
_exit_IssueFmiCmd:
  return Status;
}


EFI_STATUS
EFIAPI
FmiAuthIBB (
  IN UINT8 * IBBBuffer,
  IN UINT32 IBBSize  //assume IBBSize is in the unit of KB
  )
{
  EFI_STATUS Status;
  FMI_COMMAND *fmiCmd;
  UINT32     FmiRawStatus;
  UINT32     FmiErr;
  UINT32     FmiCommandStatus;
  DEBUG((EFI_D_INFO, "FmiAuthIBB Entry\r\n"));
  if(CmdBuffer == (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    DEBUG((EFI_D_INFO, "FmiInit must be called before\r\n"));
    Status = EFI_DEVICE_ERROR;
    goto _exit_FmiAuthIBB;
  }
  fmiCmd =  (FMI_COMMAND *)(UINTN)CmdBuffer;
  fmiCmd->CommandTag  = FMI_COMMAND_IBB_TAG;
  fmiCmd->CommandSize = FMI_COMMAND_IBB_SIZE;
  fmiCmd->CommandOpcode = FMI_COMMAND_IBB_OPCODE;
  if( ((UINT64)(UINTN) IBBBuffer < (UINT64) 0x100000000) &&
      (((UINTN)IBBBuffer + IBBSize) % 4 == 0)
    ) {
    *(UINTN *)(&fmiCmd->CommandBuffer[0]) = (UINTN)IBBBuffer + IBBSize - 1;
  } else {
    DEBUG((EFI_D_INFO, "Reallocated memory for IBBBuffer  : 0x%016Lx\r\n", (UINT64)(UINTN)IBBBuffer));
    IBBPages = EFI_SIZE_TO_PAGES (IBBSize);
    ReallocIBBBuffer = (EFI_PHYSICAL_ADDRESS)0xFFFFFFFF;
    Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  IBBPages,
                  &ReallocIBBBuffer
               );
    DEBUG((EFI_D_INFO, "Reallocated memory for IBBBuffer at: 0x%016Lx\r\n", (UINT64)(UINTN)ReallocIBBBuffer));
    if(EFI_ERROR(Status)) {
      ReallocIBBBuffer = (EFI_PHYSICAL_ADDRESS)(UINTN) NULL;
      goto _exit_FmiAuthIBB;
    }
    CopyMem((void *)(UINTN)ReallocIBBBuffer, (void *)IBBBuffer, IBBSize);
    *(UINT32 *)(&fmiCmd->CommandBuffer[0]) = (UINT32)ReallocIBBBuffer + IBBSize - 1;
  }
  Status = IssueFmiCmd();
  if(EFI_ERROR(Status)) {
    goto _exit_FmiAuthIBB;
  }
//<EIP150193*> >>>
//  FmiRawStatus     = FMI_GetRawStatus; // Need to check !!!
  FmiRawStatus = 0;
//<EIP150193*> <<<
  DEBUG((EFI_D_INFO, "FmiAuthIBB FMI status register: %r\r\n", (UINT32)FmiRawStatus));
  FmiErr           = FMI_GetError(FmiRawStatus);
  FmiCommandStatus = FMI_GetCommandStatus(FmiRawStatus);
  if(FmiErr != FMI_ERROR_NO_ERROR) {
    DEBUG((EFI_D_INFO, "Command Error: %d\r\n", FmiErr));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiAuthIBB;
  }
  if(FmiCommandStatus != FMI_STATUS_FMI_CMD_STATUS_NO_ERR) {
    DEBUG((EFI_D_INFO, "Command Status: 0x%08x\r\n", FmiCommandStatus));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiAuthIBB;
  }
  if(fmiCmd->CommandTag != FMI_RESPONSE_IBB_TAG ||
      fmiCmd->CommandSize != FMI_RESPONSE_IBB_SIZE ||
      fmiCmd->CommandOpcode != FMI_RESPONSE_IBB_OPCODE ) {
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiAuthIBB;
  }
_exit_FmiAuthIBB:
  DEBUG((EFI_D_INFO, "FmiAuthIBB exit with status: %r\r\n", Status));
  return Status;
}

EFI_STATUS
EFIAPI
FmiRSAVerify(
  IN UINT32 pkcs,
  IN UINT8 *modulus,
  IN UINT32 public_e,
  IN UINT8 *signature,
  IN UINT8 *digest
  )
{
  EFI_STATUS Status;
  FMI_RSA_COMMAND *fmiCmd;
  UINT32     FmiRawStatus;
  UINT32     FmiErr;
  UINT32     FmiCommandStatus;
  DEBUG((EFI_D_INFO, "FmiRSAVerify Entry\r\n"));
  if(CmdBuffer == (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    DEBUG((EFI_D_INFO, "FmiInit must be called before\r\n"));
    Status = EFI_DEVICE_ERROR;
    goto _exit_FmiRSAVerify;
  }
  fmiCmd =  (FMI_RSA_COMMAND *)(UINTN)CmdBuffer;
  fmiCmd->CommandTag  = FMI_COMMAND_RSA_TAG;
  fmiCmd->CommandSize = FMI_COMMAND_RSA_SIZE;
  fmiCmd->CommandOpcode = FMI_COMMAND_RSA_OPCODE;
  fmiCmd->RSA_PKCS = pkcs;
  CopyMem(&fmiCmd->RSA_MODULUS[0], modulus, 256);
  fmiCmd->RSA_e = public_e;
  CopyMem(&fmiCmd->RSA_SIGNATURE[0], signature, 256);
  CopyMem(&fmiCmd->RSA_Digest[0], digest, 32);

  Status = IssueFmiCmd();
  if(EFI_ERROR(Status)) {
    goto _exit_FmiRSAVerify;
  }
  FmiRawStatus     = FMI_GetRawStatus;
  DEBUG((EFI_D_INFO, "FmiRSAVerify FMI status register: %r\r\n", (UINT32)FmiRawStatus));
  FmiErr           = FMI_GetError(FmiRawStatus);
  FmiCommandStatus = FMI_GetCommandStatus(FmiRawStatus);
  if(FmiErr != FMI_ERROR_NO_ERROR) {
    DEBUG((EFI_D_INFO, "Command Error: %d\r\n", FmiErr));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiRSAVerify;
  }
  if(FmiCommandStatus != FMI_STATUS_FMI_CMD_STATUS_NO_ERR) {
    DEBUG((EFI_D_INFO, "Command Status: 0x%08x\r\n", FmiCommandStatus));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiRSAVerify;
  }
  if(fmiCmd->CommandTag != FMI_RESPONSE_RSA_TAG ||
      fmiCmd->CommandSize != FMI_RESPONSE_RSA_SIZE ||
      fmiCmd->CommandOpcode != FMI_RESPONSE_RSA_OPCODE ) {
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiRSAVerify;
  }
_exit_FmiRSAVerify:
  DEBUG((EFI_D_INFO, "FmiRSAVerify exit with status: %r\r\n", Status));
  return Status;
}

EFI_STATUS
EFIAPI
FmiDebugCtrlDisable(
  VOID
  )
{
  EFI_STATUS Status;
  FMI_COMMAND *fmiCmd;
  UINT32     FmiRawStatus;
  UINT32     FmiErr;
  UINT32     FmiCommandStatus;
  DEBUG((EFI_D_INFO, "FmiDebugCtrlDisable Entry\r\n"));
  if(CmdBuffer == (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    DEBUG((EFI_D_INFO, "FmiInit must be called before\r\n"));
    Status = EFI_DEVICE_ERROR;
    goto _exit_FmiDebugCtrlDisable;
  }
  fmiCmd =  (FMI_COMMAND *)(UINTN)CmdBuffer;
  fmiCmd->CommandTag  = FMI_COMMAND_DebugCtrlDisable_TAG;
  fmiCmd->CommandSize = FMI_COMMAND_DebugCtrlDisable_SIZE;
  fmiCmd->CommandOpcode = FMI_COMMAND_DebugCtrlDisable_OPCODE;
  Status = IssueFmiCmd();
  if(EFI_ERROR(Status)) {
    goto _exit_FmiDebugCtrlDisable;
  }
  FmiRawStatus     = FMI_GetRawStatus;
  DEBUG((EFI_D_INFO, "FmiDebugCtrlDisable FMI status register: %r\r\n", (UINT32)FmiRawStatus));
  FmiErr           = FMI_GetError(FmiRawStatus);
  FmiCommandStatus = FMI_GetCommandStatus(FmiRawStatus);
  if(FmiErr != FMI_ERROR_NO_ERROR) {
    DEBUG((EFI_D_INFO, "Command Error: %d\r\n", FmiErr));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiDebugCtrlDisable;
  }
  if(FmiCommandStatus != FMI_STATUS_FMI_CMD_STATUS_NO_ERR) {
    DEBUG((EFI_D_INFO, "Command Status: 0x%08x\r\n", FmiCommandStatus));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiDebugCtrlDisable;
  }
  if(fmiCmd->CommandTag != FMI_RESPONSE_DebugCtrlDisable_TAG ||
      fmiCmd->CommandSize != FMI_RESPONSE_DebugCtrlDisable_SIZE ||
      fmiCmd->CommandOpcode != FMI_RESPONSE_DebugCtrlDisable_OPCODE ) {
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiDebugCtrlDisable;
  }
_exit_FmiDebugCtrlDisable:
  DEBUG((EFI_D_INFO, "FmiDebugCtrlDisable exit with status: %r\r\n", Status));
  return Status;


}

EFI_STATUS
EFIAPI
FmiSetDebugState(
  IN UINT32 state
  )
{
  EFI_STATUS Status;
  FMI_SETDSTATE_COMMAND *fmiCmd;
  UINT32     FmiRawStatus;
  UINT32     FmiErr;
  UINT32     FmiCommandStatus;
  DEBUG((EFI_D_INFO, "FmiSetDebugState Entry\r\n"));
  if(CmdBuffer == (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    DEBUG((EFI_D_INFO, "FmiInit must be called before\r\n"));
    Status = EFI_DEVICE_ERROR;
    goto _exit_FmiSetDebugState;
  }
  fmiCmd =  (FMI_SETDSTATE_COMMAND *)(UINTN)CmdBuffer;
  fmiCmd->CommandTag  = FMI_COMMAND_SetDState_TAG;
  fmiCmd->CommandSize = FMI_COMMAND_SetDState_SIZE;
  fmiCmd->CommandOpcode = FMI_COMMAND_SetDState_OPCODE;
  fmiCmd->State = state;
  Status = IssueFmiCmd();
  if(EFI_ERROR(Status)) {
    goto _exit_FmiSetDebugState;
  }
  FmiRawStatus     = FMI_GetRawStatus;
  DEBUG((EFI_D_INFO, "FmiSetDebugState FMI status register: %r\r\n", (UINT32)FmiRawStatus));
  FmiErr           = FMI_GetError(FmiRawStatus);
  FmiCommandStatus = FMI_GetCommandStatus(FmiRawStatus);
  if(FmiErr != FMI_ERROR_NO_ERROR) {
    DEBUG((EFI_D_INFO, "Command Error: %d\r\n", FmiErr));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiSetDebugState;
  }
  if(FmiCommandStatus != FMI_STATUS_FMI_CMD_STATUS_NO_ERR) {
    DEBUG((EFI_D_INFO, "Command Status: 0x%08x\r\n", FmiCommandStatus));
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiSetDebugState;
  }
  if(fmiCmd->CommandTag != FMI_RESPONSE_SetDState_TAG ||
      fmiCmd->CommandSize != FMI_RESPONSE_SetDState_SIZE ||
      fmiCmd->CommandOpcode != FMI_RESPONSE_SetDState_OPCODE) {
    Status =  EFI_DEVICE_ERROR;
    goto _exit_FmiSetDebugState;
  }
_exit_FmiSetDebugState:
  DEBUG((EFI_D_INFO, "FmiDebugCtrlDisable exit with status: %r\r\n", Status));
  return Status;
}

EFI_STATUS
EFIAPI
FreeFmiResource(
  VOID
  )
{
  if(CmdBuffer != (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    gBS->FreePages( CmdBuffer,
           NrPages
           );
  }
  if(ReallocIBBBuffer != (EFI_PHYSICAL_ADDRESS)(UINTN) NULL) {
    gBS->FreePages( ReallocIBBBuffer,
           IBBPages
           );
  }

  return EFI_SUCCESS;
}
