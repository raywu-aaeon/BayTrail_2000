/*++

Copyright (c)  2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Fmi.h

--*/



#ifndef _FMI_H_
#define _FMI_H_

#include "SeCChipset.h"
#include <PchAccess.h>
#include <Uefi/UefiBaseType.h>
#include <Library/PciLib.h>
#include <MmioAccess.h>

#define PMC_BASE_ADDRESS                  0xFED03000 // PMC Memory Base Address

typedef struct {
  UINT32 CommandTag;
  UINT32 CommandSize;
  UINT32 CommandOpcode;
  UINT8  CommandBuffer[1];
} FMI_COMMAND;

typedef struct {
  UINT32 CommandTag;
  UINT32 CommandSize;
  UINT32 CommandOpcode;
  UINT32 RSA_PKCS;
  UINT8  RSA_MODULUS[256];
  UINT32 RSA_e;
  UINT8  RSA_SIGNATURE[256];
  UINT8  RSA_Digest[32];
} FMI_RSA_COMMAND;

typedef struct {
  UINT32 CommandTag;
  UINT32 CommandSize;
  UINT32 CommandOpcode;
  UINT32 State;
} FMI_SETDSTATE_COMMAND;


#define FMI_STATUS                                0x0054     //PCI OFFSET
  #define FMI_STATUS_STATE_MASK                   0x0000000F
  #define FMI_STATUS_STATE_NOT_READY              0x0
  #define FMI_STATUS_STATE_READY                  0x1
  #define FMI_STATUS_STATE_IN_ERROR               0x2
  #define FMI_STATUS_STATE_CLOSE                  0x4
  #define FMI_STATUS_STATE_RESERVED               0x8
  
  #define FMI_STATUS_FLIP_MASK                    0x000000F0
  
  #define FMI_STATUS_BUFFER_SIZE_POS              8
  #define FMI_STATUS_BUFFER_SIZE_MASK             0x00000F00
  
  #define FMI_STATUS_FMI_ERROR_POS                12
  #define FMI_STATUS_FMI_ERROR_MASK               0x0000F000  
  #define FMI_ERROR_NO_ERROR                      0x0
  #define FMI_ERROR_FMI_MUST_INIT                 0x1
  #define FMI_ERROR_FMI_BAD_SATT                  0x2
  #define FMI_ERROR_FMI_NOT_ACCESS                0x3
  #define FMI_ERROR_FMI_TOO_SHORT                 0x4
  #define FMI_ERROR_FMI_BAD_ALIGNMENT             0x5
  
  #define FMI_STATUS_FMI_CMD_STATUS_POS           16
  #define FMI_STATUS_FMI_CMD_STATUS_MASK          0xFFFF0000
  #define FMI_STATUS_FMI_CMD_STATUS_NO_ERR        0x0
  
#define FMI_ICR_BAR1_OFFSET                       0x2158    //BAR1 OFFSET
  #define FMI_ICR_FMI_INIT                        0x00000000
  #define FMI_ICR_FMI_SHUTDOWN                    0xFFFFFFFF
#define FMI_SATT23_BAR1_OFFSET                    0x1800    //BAR1 OFFSET
  #define EN_WR_SATT3                             0x1
  #define EN_WR_SATT2                             0x2


#define  FMI_COMMAND_TAG                  0x7
#define  FMI_RESPONSE_TAG                 0xC
#define  FMI_COMMAND_IBB_TAG              0x7
#define  FMI_COMMAND_IBB_SIZE             0x10
#define  FMI_COMMAND_IBB_OPCODE           0x1
#define  FMI_RESPONSE_IBB_TAG             0xC
#define  FMI_RESPONSE_IBB_SIZE            0xC
#define  FMI_RESPONSE_IBB_OPCODE          0x1

#define  FMI_COMMAND_RSA_TAG              0x7
#define  FMI_COMMAND_RSA_SIZE             0x234
#define  FMI_COMMAND_RSA_OPCODE           0x2
#define  FMI_RESPONSE_RSA_TAG             0xC
#define  FMI_RESPONSE_RSA_SIZE            0xC
#define  FMI_RESPONSE_RSA_OPCODE          0x2

#define  FMI_COMMAND_SetDState_TAG              0x7
#define  FMI_COMMAND_SetDState_SIZE             0x10
#define  FMI_COMMAND_SetDState_OPCODE           0x3
#define  FMI_RESPONSE_SetDState_TAG             0xC
#define  FMI_RESPONSE_SetDState_SIZE            0xC
#define  FMI_RESPONSE_SetDState_OPCODE          0x3

#define  FMI_COMMAND_DebugCtrlDisable_TAG              0x7
#define  FMI_COMMAND_DebugCtrlDisable_SIZE             0xC
#define  FMI_COMMAND_DebugCtrlDisable_OPCODE           0x4
#define  FMI_RESPONSE_DebugCtrlDisable_TAG             0xC
#define  FMI_RESPONSE_DebugCtrlDisable_SIZE            0xC
#define  FMI_RESPONSE_DebugCtrlDisable_OPCODE          0x4



#define FmiPciRead32(Register) PciRead32 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register))
#define FmiPciWrite32(Register, Data) \
  PciWrite32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )
#define FMI_GET_STATE  (FmiPciRead32(FMI_STATUS) & FMI_STATUS_STATE_MASK)
#define FMI_GET_FLIP   (FmiPciRead32(FMI_STATUS) & FMI_STATUS_FLIP_MASK)
#define FMI_GET_ERROR  ((FmiPciRead32(FMI_STATUS) & FMI_STATUS_FMI_ERROR_MASK) >> FMI_STATUS_FMI_ERROR_POS)
#define FMI_GET_BUFFER_SIZE ((FmiPciRead32(FMI_STATUS) & FMI_STATUS_BUFFER_SIZE_MASK) >> FMI_STATUS_BUFFER_SIZE_POS)
#define FMI_GET_COMMAND_STATUS ((FmiPciRead32(FMI_STATUS) & FMI_STATUS_FMI_CMD_STATUS_MASK) >> FMI_STATUS_FMI_CMD_STATUS_POS)

#define FMI_GetRawStatus   FmiPciRead32(FMI_STATUS)
#define FMI_GetState(RawStatus) (RawStatus & FMI_STATUS_STATE_MASK)
#define FMI_GetFlip(RawStatus)  (RawStatus & FMI_STATUS_FLIP_MASK)
#define FMI_GetError(RawStatus)  ((RawStatus & FMI_STATUS_FMI_ERROR_MASK) >> FMI_STATUS_FMI_ERROR_POS)
#define FMI_GetBufferSize(RawStatus) ((RawStatus & FMI_STATUS_BUFFER_SIZE_MASK) >> FMI_STATUS_BUFFER_SIZE_POS)
#define FMI_GetCommandStatus(RawStatus) ((RawStatus & FMI_STATUS_FMI_CMD_STATUS_MASK) >> FMI_STATUS_FMI_CMD_STATUS_POS)


#define FMI_WRITE_ICR(val)   Mmio32(FmiPciRead32(0x14), FMI_ICR_BAR1_OFFSET) = (val)
#define FMI_ENABLE_SATT23    Mmio32Or(FmiPciRead32(0x14), FMI_SATT23_BAR1_OFFSET, EN_WR_SATT3 | EN_WR_SATT2)
#define FMI_DISABLE_SATT23   Mmio32And(FmiPciRead32(0x14), FMI_SATT23_BAR1_OFFSET, ~(EN_WR_SATT3 | EN_WR_SATT2))

#endif
