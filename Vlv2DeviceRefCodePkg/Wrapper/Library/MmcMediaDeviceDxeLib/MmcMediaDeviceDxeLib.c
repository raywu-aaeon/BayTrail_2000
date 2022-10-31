/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/*++
Module Name:

  MmcMediaDeviceDxeLib.c

Abstract:

  MMC/SD specific functions library

--*/

#include "MmcMediaDeviceDxeLib.h"

EFI_STATUS
CheckCardStatus (
  IN  UINT32    Status
  )
/*++

  Routine Description:
    Check card status, print the debug info and check the error
  
  Arguments:
    Status - Status got from card status register

  Returns:  
    EFI_DEVICE_ERROR
    EFI_SUCCESS
 --*/   
{
  CARD_STATUS    *CardStatus;
  CardStatus = (CARD_STATUS*)(&Status);

  if (CardStatus->ADDRESS_OUT_OF_RANGE) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ADDRESS_OUT_OF_RANGE\n"));
  }
  
  if (CardStatus->ADDRESS_MISALIGN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ADDRESS_MISALIGN\n"));
  }

  if (CardStatus->BLOCK_LEN_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: BLOCK_LEN_ERROR\n"));
  }

  if (CardStatus->ERASE_SEQ_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_SEQ_ERROR\n"));
  }

  if (CardStatus->ERASE_PARAM) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_PARAM\n"));
  }

  if (CardStatus->WP_VIOLATION) {
    DEBUG ((EFI_D_ERROR, "CardStatus: WP_VIOLATION\n"));
  }
  
  if (CardStatus->CARD_IS_LOCKED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CARD_IS_LOCKED\n"));
  }

  if (CardStatus->LOCK_UNLOCK_FAILED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: LOCK_UNLOCK_FAILED\n"));
  }

  if (CardStatus->COM_CRC_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: COM_CRC_ERROR\n"));
  }

  if (CardStatus->ILLEGAL_COMMAND) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ILLEGAL_COMMAND\n"));
  }

  if (CardStatus->CARD_ECC_FAILED) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CARD_ECC_FAILED\n"));
  }

  if (CardStatus->CC_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CC_ERROR\n"));
  }

  if (CardStatus->ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERROR\n"));
  }

  if (CardStatus->UNDERRUN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: UNDERRUN\n"));
  }

  if (CardStatus->OVERRUN) {
    DEBUG ((EFI_D_ERROR, "CardStatus: OVERRUN\n"));
  }

  if (CardStatus->CID_CSD_OVERWRITE) {
    DEBUG ((EFI_D_ERROR, "CardStatus: CID_CSD_OVERWRITE\n"));
  }

  if (CardStatus->WP_ERASE_SKIP) {
    DEBUG ((EFI_D_ERROR, "CardStatus: WP_ERASE_SKIP\n"));
  }

  if (CardStatus->ERASE_RESET) {
    DEBUG ((EFI_D_ERROR, "CardStatus: ERASE_RESET\n"));
  }

  if (CardStatus->SWITCH_ERROR) {
    DEBUG ((EFI_D_ERROR, "CardStatus: SWITCH_ERROR\n"));
  }

  if ((Status & 0xFCFFA080) != 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SendCommand (
  IN   EFI_SD_HOST_IO_PROTOCOL    *This,
  IN   UINT16                     CommandIndex,
  IN   UINT32                     Argument,
  IN   TRANSFER_TYPE              DataType,
  IN   UINT8                      *Buffer, OPTIONAL
  IN   UINT32                     BufferSize,    
  IN   RESPONSE_TYPE              ResponseType,
  IN   UINT32                     TimeOut,  
  OUT  UINT32                     *ResponseData
  )
/*++

  Routine Description:
    Send command by using Host IO protocol
  
  Arguments:
    This           - Pointer to EFI_SD_HOST_IO_PROTOCOL
    CommandIndex   - The command index to set the command index field of command register
    Argument       - Command argument to set the argument field of command register
    DataType       - TRANSFER_TYPE, indicates no data, data in or data out
    Buffer         - Contains the data read from / write to the device
    BufferSize     - The size of the buffer
    ResponseType   - RESPONSE_TYPE
    TimeOut        - Time out value in 1 ms unit
    ResponseData   - Depending on the ResponseType, such as CSD or card status

  Returns:  
    EFI_INVALID_PARAMETER
    EFI_UNSUPPORTED
    EFI_DEVICE_ERROR
    EFI_SUCCESS
 --*/   
{

  EFI_STATUS    Status;

  Status = This->SendCommand (
                   This,
                   CommandIndex,
                   Argument,
                   DataType,
                   Buffer,
                   BufferSize,
                   ResponseType,
                   TimeOut,
                   ResponseData
                   );
  if (!EFI_ERROR (Status)) {
    if (ResponseType == ResponseR1 || ResponseType == ResponseR1b) {
      ASSERT(ResponseData != NULL);
      Status = CheckCardStatus (*ResponseData);
    }
  } else {
    This->ResetSdHost (This, Reset_DAT_CMD);
  }

  return Status;
}

EFI_STATUS
MmcSelect (
  IN CARD_DATA     *CardData,
  IN BOOLEAN       Select
  )
{
  return SendCommand (
           CardData->SdHostIo,
           SELECT_DESELECT_CARD,
           Select ? (CardData->Address << 16) : ~(CardData->Address << 16),
           NoData,
           NULL,
           0,
           ResponseR1,
           TIMEOUT_COMMAND,
           (UINT32*)&(CardData->CardStatus)
           );
}

EFI_STATUS
MmcSendSwitch (
  IN  CARD_DATA              *CardData,
  IN SWITCH_ARGUMENT         *SwitchArgument
  )
{
  EFI_STATUS                 Status;
  EFI_SD_HOST_IO_PROTOCOL    *SdHostIo;

  SdHostIo = CardData->SdHostIo;

  Status  = SendCommand (
              SdHostIo,
              SWITCH,
              *(UINT32*)SwitchArgument,
              NoData,
              NULL,
              0,
              ResponseR1b,
              TIMEOUT_DATA,
              (UINT32*)&(CardData->CardStatus)
              );
  if (!EFI_ERROR (Status)) {
     Status  = SendCommand (
                 SdHostIo,
                 SEND_STATUS,
                 (CardData->Address << 16),
                 NoData,
                 NULL,
                 0,
                 ResponseR1,
                 TIMEOUT_COMMAND,
                 (UINT32*)&(CardData->CardStatus)
                 );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "SWITCH FAILURE\n"));
    }
  }

  return Status;
}

EFI_STATUS
MmcUpdateCardStatus (
  IN CARD_DATA     *CardData
  )
{
  return SendCommand (
           CardData->SdHostIo,
           SEND_STATUS,
           (CardData->Address << 16),
           NoData,
           NULL,
           0,
           ResponseR1,
           TIMEOUT_COMMAND,
           (UINT32*)&(CardData->CardStatus)
           );
}

EFI_STATUS
MmcMoveToTranState (
  IN CARD_DATA     *CardData
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  if (CardData->CardStatus.CURRENT_STATE != Tran_STATE) {
    //
    // Put the card into tran state
    //
    Status = MmcSelect (CardData, TRUE);
    DEBUG((EFI_D_INFO, "MmcMoveToTranState: CMD7 -> %r\n", Status));
    MmcUpdateCardStatus (CardData);
  }

  if (CardData->CardStatus.CURRENT_STATE != Tran_STATE) {
    DEBUG((EFI_D_ERROR, "MmcMoveToTranState: Unable to put card into tran state\n"));
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

EFI_STATUS
MmcReadExtCsd (
  IN CARD_DATA     *CardData
  )
{
  EFI_STATUS Status;

  Status = MmcMoveToTranState (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status  = SendCommand (
              CardData->SdHostIo,
              SEND_EXT_CSD,
              (CardData->Address << 16),
              InData,
              CardData->AlignedBuffer,
              sizeof (EXT_CSD),
              ResponseR1,
              TIMEOUT_DATA,
              (UINT32*)&(CardData->CardStatus)
              );
  DEBUG ((EFI_D_INFO, "MmcReadExtCsd: SEND_EXT_CSD -> %r\n", Status));
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  CopyMem (&(CardData->ExtCSDRegister), CardData->AlignedBuffer, sizeof (EXT_CSD));

  return Status;
}

EFI_STATUS
MmcSetExtCsd8 (
  IN  CARD_DATA              *CardData,
  IN  UINT8                  Index,
  IN  UINT8                  Value
  )
{
  EFI_STATUS                 Status;
  SWITCH_ARGUMENT            SwitchArgument;

  Status = MmcMoveToTranState (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem(&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = (UINT8) Value;
  SwitchArgument.Index  = (UINT8) Index;
  SwitchArgument.Access = WriteByte_Mode; // SetBits_Mode;
  return MmcSendSwitch (CardData, &SwitchArgument);
}

EFI_STATUS
MmcSetExtCsd24 (
  IN  CARD_DATA              *CardData,
  IN  UINT8                  Index,
  IN  UINT32                 Value
  )
{
  EFI_STATUS                 Status;
  UINTN                      Loop;

  ASSERT ((Value & 0xff000000ULL) == 0);

  for (Loop = 0; Loop < 3; Loop++) {
    Status = MmcSetExtCsd8 (CardData, Index + (UINT8)Loop, Value & 0xff);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Value = Value >> 8;
  }

  return Status;
}

UINT32
MmcGetExtCsd8 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  )
{
  ASSERT (Offset < sizeof (CardData->ExtCSDRegister));
  return ((UINT8*)&CardData->ExtCSDRegister)[Offset];
}

UINT32
MmcGetExtCsd32 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  )
{
  return *(UINT32*) (((UINT8*)&CardData->ExtCSDRegister) + Offset);
}

UINT32
MmcGetExtCsd24 (
  IN CARD_DATA                        *CardData,
  IN UINTN                            Offset
  )
{
  return MmcGetExtCsd32 (CardData, Offset) & 0xffffff;
}

UINTN
MmcGetCurrentPartitionNum (
  IN  CARD_DATA              *CardData
  )
{
  return MmcGetExtCsd8 (
           CardData,
           OFFSET_OF (EXT_CSD, PARTITION_CONFIG)
           ) & 0x7;
}

EFI_STATUS
MmcSelectPartitionNum (
  IN  CARD_DATA              *CardData,
  IN  UINT8                  Partition
  )
{
  EFI_STATUS  Status;
  UINTN       Offset;
  UINT8       *ExtByte;
  UINTN       CurrentPartition;

  if (Partition > 7) {
    return EFI_INVALID_PARAMETER;
  }
  
  CurrentPartition = MmcGetCurrentPartitionNum (CardData);
  if (Partition == CurrentPartition) {
    return EFI_SUCCESS;
  }

  Status = MmcReadExtCsd (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((EFI_D_INFO,
    "MmcSelectPartitionNum: Switch partition: %d => %d\n",
    CurrentPartition,
    Partition
    ));

  Offset = OFFSET_OF (EXT_CSD, PARTITION_CONFIG);
  Status = MmcSetExtCsd8 (CardData, (UINT8)Offset, Partition);

  Status = MmcReadExtCsd (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CurrentPartition = MmcGetCurrentPartitionNum (CardData);
  if (Partition != CurrentPartition) {
    DEBUG ((EFI_D_INFO, "MmcSelectPartitionNum: Switch partition failed!\n"));
    return EFI_DEVICE_ERROR;
  }

  ExtByte = NULL;

  return Status;
}

EFI_STATUS
MmcSelectPartition (
  IN  MMC_PARTITION_DATA     *Partition
  )
{
  return MmcSelectPartitionNum (
           Partition->CardData,
           (UINT8)CARD_DATA_PARTITION_NUM (Partition)
           );
}


