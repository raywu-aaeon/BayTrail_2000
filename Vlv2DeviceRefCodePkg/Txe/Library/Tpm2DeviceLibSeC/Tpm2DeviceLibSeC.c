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

#include <Uefi.h>
#include <Protocol/Hash.h>
//#include <Library/PeiServicesLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <SeCAccess.h>
#include <Library/PciLib.h>
#include <HeciRegs.h>
#include <PttHciRegs.h>
#include <PttHciDeviceLib.h>
#include <IndustryStandard/Tpm20.h>
#include <Library/TimerLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
UINT32 mPttBaseAddress = 0;

BOOLEAN
isSeCExpose(
  )
/*++

Routine Description:

  This procedure will check the exposure of SeC device.

Arguments:

    PeiServices - Pointer to the PEI services table
Returns:

  Return EFI_SUCCESS

--*/
{
  BOOLEAN Result;

  // Device ID read here
  UINT32 DeviceID;

  Result = FALSE;

  DeviceID = (HeciPciRead32 (R_SEC_DevID_VID) & S_SEC_DevID_MASK) >> 16;

  if(DeviceID >= S_SEC_DevID_RANGE_LO && DeviceID <= S_SEC_DevID_RANGE_HI) {
    Result = TRUE;
    DEBUG ((EFI_D_INFO, "SeC Device ID: %x\n", DeviceID));
  }
  return Result;
}

/**
  Get the PTT Base Address
**/
UINT32
getfTPMBaseAddr(void)
{
  UINT32 sattLsb;
  UINT32 sattMsb;
  UINT32 sattCtrl;
  UINT32 SeCsattCtrl;
  HECI_FWS_REGISTER       SeCFirmwareStatus;

  if (!isSeCExpose()) {
    DEBUG((EFI_D_ERROR, "SeC is not Exposed, Disable fTPM\n"));
    return 0;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  DEBUG ((EFI_D_ERROR, "R_SEC_FW_STS0 is %08x %x\n", SeCFirmwareStatus.ul, SeCFirmwareStatus.r.SeCOperationMode));
  if(SeCFirmwareStatus.r.SeCOperationMode != 0) {
    DEBUG((EFI_D_ERROR, "SeC is not in Normal mode, disable fTPM\n"));
    return 0;
  }

  SeCsattCtrl = HeciPciRead32(R_SATT1_CTRL);
  sattLsb = HeciPciRead32(R_SATT_PTT_BRG_BA_LSB);
  sattMsb = (HeciPciRead32(R_SATT_PTT_CTRL) >> 8) & 0x0f;
  sattCtrl = HeciPciRead32(R_SATT_PTT_CTRL);
  DEBUG((EFI_D_ERROR, "******  fTPM Info ************ \n"));
  DEBUG((EFI_D_ERROR, "R_SATT_PTT_BRG_BA_LSB offset %x Value %x\n", R_SATT_PTT_BRG_BA_LSB, sattLsb));
  DEBUG((EFI_D_ERROR, "R_SATT_PTT_CTRL offset %x %x\n", R_SATT_PTT_CTRL, sattCtrl));
  DEBUG((EFI_D_ERROR, "R_SATT1_CTRL offset %x %x\n", R_SATT1_CTRL, SeCsattCtrl));

  if ((SeCsattCtrl & B_ENTRY_VLD) != B_ENTRY_VLD) {
    DEBUG ((EFI_D_ERROR, "Error! SeC is not enabled \n"));
    return 0;
  }

  if ((sattCtrl & B_ENTRY_VLD) != B_ENTRY_VLD) {
    DEBUG ((EFI_D_ERROR, "Error! fTPM is not enabled \n"));
    return 0;
  }

  if (sattLsb == 0) {
    DEBUG ((EFI_D_ERROR, "Error! PTT Base Address Should not be 0 \n"));
    return 0;
  }
  if(sattMsb != 0) {
    DEBUG ((EFI_D_ERROR, "Error! no 64 bits support"));
    return 0;
  }
  return sattLsb;
}

/**
  The constructor function caches the pointer to PEI services.

  The constructor function caches the pointer to PEI services.
  It will always return EFI_SUCCESS.

  @param  FfsHeader   Pointer to FFS header the loaded driver.
  @param  PeiServices Pointer to the PEI services.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
Tpm2DeviceLibConstructor (
  VOID
  )
{
  mPttBaseAddress = getfTPMBaseAddr();
  return EFI_SUCCESS;
}

/**

  Prints command or response buffer for debugging purposes.

  @param[in] Buffer     Buffer to print.
  @param[in] BufferSize Buffer data length.

**/
VOID
EFIAPI
PttHciPrintBuffer(IN UINT8 *Buffer, IN UINT32 BufferSize)
{
  UINT8 Index;

  DEBUG ((EFI_D_INFO, "Buffer Address: 0x%08x, Size: 0x%08x, Value:\n", Buffer, BufferSize));
  for(Index = 0; Index < BufferSize; Index++) {
    DEBUG ((EFI_D_INFO, "%02x ", *(Buffer + Index)));
    if((Index + 1) % 16 == 0) DEBUG ((EFI_D_INFO, "\n"));
    if (Index >= 32) break;
  }
  DEBUG ((EFI_D_INFO, "\n"));
}

/**
  Copy data from the MMIO region to system memory by using 32-bit access.

  Copy data from the MMIO region specified by starting address StartAddress
  to system memory specified by Buffer by using 32-bit access. The total
  number of byte to be copied is specified by Length. Buffer is returned.

  If StartAddress is not aligned on a 32-bit boundary, then ASSERT().

  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  If Length is not aligned on a 32-bit boundary, then ASSERT().
  If Buffer is not aligned on a 32-bit boundary, then ASSERT().

  @param  StartAddress    The starting address for the MMIO region to be copied from.
  @param  Length          The size, in bytes, of Buffer.
  @param  Buffer          The pointer to a system memory buffer receiving the data read.

  @return Buffer

**/
UINT32 *
EFIAPI
MmioReadBuffer32 (
  IN  UINTN       StartAddress,
  IN  UINTN       Length,
  OUT UINT32      *Buffer
  )
{
  UINT32    *ReturnBuffer;

  ASSERT ((StartAddress & (sizeof (UINT32) - 1)) == 0);

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));

  ASSERT ((Length & (sizeof (UINT32) - 1)) == 0);
  ASSERT (((UINTN) Buffer & (sizeof (UINT32) - 1)) == 0);

  ReturnBuffer = Buffer;

  while (Length != 0) {
    *(Buffer++) = MmioRead32 (StartAddress);
    StartAddress += sizeof (UINT32);
    Length -= sizeof (UINT32);
  }

  return ReturnBuffer;
}

/**
  Copy data from system memory to the MMIO region by using 32-bit access.

  Copy data from system memory specified by Buffer to the MMIO region specified
  by starting address StartAddress by using 32-bit access. The total number
  of byte to be copied is specified by Length. Buffer is returned.

  If StartAddress is not aligned on a 32-bit boundary, then ASSERT().

  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS -Buffer + 1), then ASSERT().

  If Length is not aligned on a 32-bit boundary, then ASSERT().

  If Buffer is not aligned on a 32-bit boundary, then ASSERT().

  @param  StartAddress    The starting address for the MMIO region to be copied to.
  @param  Length          The size, in bytes, of Buffer.
  @param  Buffer          The pointer to a system memory buffer containing the data to write.

  @return Buffer

**/
UINT32 *
EFIAPI
MmioWriteBuffer32 (
  IN  UINTN        StartAddress,
  IN  UINTN        Length,
  IN  CONST UINT32 *Buffer
  )
{
  UINT32    *ReturnBuffer;

  ASSERT ((StartAddress & (sizeof (UINT32) - 1)) == 0);

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));

  ASSERT ((Length & (sizeof (UINT32) - 1)) == 0);
  ASSERT (((UINTN) Buffer & (sizeof (UINT32) - 1)) == 0);

  ReturnBuffer = (UINT32 *) Buffer;

  while (Length != 0) {
    MmioWrite32 (StartAddress, *(Buffer++));

    StartAddress += sizeof (UINT32);
    Length -= sizeof (UINT32);
  }

  return ReturnBuffer;
}

/**

  Sets FTPM_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD

**/
EFI_STATUS
EFIAPI
PttHciRequestCommandExec()
{
  EFI_STATUS Status = EFI_SUCCESS;

  DEBUG ((EFI_D_INFO, "PTT: PttHciRequestCommandExec start\n"));

  DEBUG ((EFI_D_INFO, "Command ready for processing - write 0x%08x to FTPM_CA_START register (@ 0x%08x)\n",
          V_PTT_HCI_COMMAND_AVAILABLE_START,
          (mPttBaseAddress + R_PTT_HCI_CA_START)));
  MmioWrite32((UINTN)mPttBaseAddress + R_PTT_HCI_CA_START, V_PTT_HCI_COMMAND_AVAILABLE_START);

  /*  ///
    /// Write 0x1 to HCI CMD register to indicate that a command is available for processing
    ///
    DEBUG ((EFI_D_INFO, "Command ready for processing - write 0x%08x to FTPM_CMD register (@ 0x%08x)\n",
                        V_PTT_HCI_COMMAND_AVAILABLE_CMD,
                        (mPttBaseAddress + R_PTT_HCI_CMD)));
    MmioWrite32((UINTN)mPttBaseAddress + R_PTT_HCI_CMD, V_PTT_HCI_COMMAND_AVAILABLE_CMD);
    */

  // set PPT_ICR
  HeciPciWrite32(PTT_ICR, 0x1);
  return Status;
}

/**

  Checks whether the value of a FTPM register satisfies the input BIT setting.

  @param[in]  Register     Address port of register to be checked.
  @param[in]  BitSet       Check these data bits are set.
  @param[in]  BitClear     Check these data bits are clear.
  @param[in]  TimeOut      The max wait time (unit MicroSecond) when checking register.

  @retval     EFI_SUCCESS  The register satisfies the check bit.
  @retval     EFI_TIMEOUT  The register can't run into the expected status in time.

**/
EFI_STATUS
EFIAPI
PttHciWaitRegisterBits(
  IN      EFI_PHYSICAL_ADDRESS    RegAddress,
  IN      UINT32                  BitSet,
  IN      UINT32                  BitClear,
  IN      UINT32                  TimeOut
  )
{
  UINT32 RegRead;
  UINT32 WaitTime;

  DEBUG ((EFI_D_INFO, "PTT: PttHciWaitRegisterBits start\n"));
#if (_SIMIC_ || _SLE_HYB_)
  for (WaitTime = 0; WaitTime < TimeOut; WaitTime += 0) {
#else
  for (WaitTime = 0; WaitTime < TimeOut; WaitTime += 0) {
#endif
    RegRead = MmioRead32 ((UINTN)RegAddress);
    DEBUG ((EFI_D_INFO, "RegRead: 0x%08x, BitSetMask: 0x%08x, BitClearMask: 0x%08x, WaitTime: %d (microsec)\n", RegRead, BitSet, BitClear, WaitTime));

    if ((RegRead & BitSet) == BitSet && (RegRead & BitClear) == 0) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
  }
  return EFI_TIMEOUT;
}

/**

  Sends command to FTPM for execution.

  @param[in] FtpmBuffer   Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.

**/
EFI_STATUS
EFIAPI
PttHciSend(
  IN     UINT8      *FtpmBuffer,
  IN     UINT32     DataLength
  )
{
  EFI_STATUS Status;

  DEBUG ((EFI_D_INFO, "PTT: PttHciSend start\n"));

  Status = PttHciWaitRegisterBits(
             (EFI_PHYSICAL_ADDRESS)(UINTN)(mPttBaseAddress + R_PTT_HCI_CA_START),
             V_PTT_HCI_IGNORE_BITS,
             V_PTT_HCI_ALL_BITS_CLEAR,
             PTT_HCI_TIMEOUT_A
           );

  if(EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "FTPM_CA_START register not clear - TPM2 command cannot be sent! EFI_ERROR = %r\n", Status));
    return EFI_NOT_READY;
  }
  ///
  /// Align command size to dword before writing to FTPM_CRB
  ///
  if(DataLength % 4 != 0) {
    DEBUG ((EFI_D_INFO, "Alignment: DataLength change from %d ", DataLength));
    DataLength += (4 - (DataLength % 4));
    DEBUG ((EFI_D_INFO, "to %d\n", DataLength));
  }

  MmioWriteBuffer32((UINTN)(mPttBaseAddress + R_PTT_HCI_CRB), (UINTN)DataLength, (UINT32*)FtpmBuffer);

  ///
  /// FTPM_CA_CMD - the physical address to which the TPM 2.0 driver will write the command to execute
  ///
  /*  MmioWrite32((UINTN)(mPttBaseAddress + R_PTT_HCI_CA_CMD), mPttBaseAddress + R_PTT_HCI_CRB);
    MmioWrite32((UINTN)(mPttBaseAddress + R_PTT_HCI_CA_CMD_SZ), DataLength);

    DEBUG ((EFI_D_INFO, "FTPM_CA_CMD (@ 0x%08x) written, value = 0x%08x\n",
                         (mPttBaseAddress + R_PTT_HCI_CA_CMD),
                         mPttBaseAddress + R_PTT_HCI_CRB));
    DEBUG ((EFI_D_INFO, "FTPM_CA_CMD_SZ (@ 0x%08x) written, value = 0x%08x\n",
                         (mPttBaseAddress + R_PTT_HCI_CA_CMD_SZ), DataLength));
  */
  ///
  /// Set FTPM_CMD and FTPM_CA_START registers to indicate TPM command ready for execution
  ///
  Status = PttHciRequestCommandExec();
  DEBUG_CODE(
  if(Status == EFI_SUCCESS) {
  DEBUG ((EFI_D_INFO, "FTPM_CMD register written - TPM2 command available for processing\n"));
  }
  );
  return Status;
}

/**

  Receives response data of last command from FTPM.

  @param[out] FtpmBuffer        Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.

**/
EFI_STATUS
EFIAPI
PttHciReceive(
  OUT     UINT8     *FtpmBuffer,
  OUT     UINT32    *RespSize
  )
{
  EFI_STATUS Status;
  UINT16 Data16;
  UINT32 Data32;
  UINT32 DataLength;
  DEBUG ((EFI_D_INFO, "PTT: PttHciReceive start\n"));

  ///
  /// Wait for the command completion - poll FTPM_CA_START clear
  ///
  DEBUG ((EFI_D_INFO, "PTT: Check Start status (FTPM_CA_START)\n"));
  Status = PttHciWaitRegisterBits(
             (EFI_PHYSICAL_ADDRESS)(UINTN)(mPttBaseAddress + R_PTT_HCI_CA_START),
             V_PTT_HCI_IGNORE_BITS,
             V_PTT_HCI_ALL_BITS_CLEAR,
             PTT_HCI_TIMEOUT_B
           );
  if(EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "FTPM_CA_START register not clear - TPM2 response cannot be read! EFI_ERROR = %r\n", Status));
    goto Exit;
  }

  ///
  /// Check for error condition - FTPM_CA_ERROR
  ///
  DEBUG ((EFI_D_INFO, "PTT: Check Error status (FTPM_CA_ERROR)\n"));
  Status = PttHciWaitRegisterBits(
             (EFI_PHYSICAL_ADDRESS)(UINTN)(mPttBaseAddress + R_PTT_HCI_CA_ERROR),
             V_PTT_HCI_IGNORE_BITS,
             V_PTT_HCI_ALL_BITS_CLEAR,
             PTT_HCI_TIMEOUT_A
           );
  if(EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "FTPM_CA_ERROR register set - TPM2 response cannot be provided! EFI_ERROR = %r\n", Status));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  DEBUG ((EFI_D_INFO, "FTPM_CA_START register clear - TPM2 command processing completed - ready to read\n"));

  ///
  /// Read the response data header
  ///
  MmioReadBuffer32((UINTN)mPttBaseAddress + R_PTT_HCI_CRB, PTT_HCI_RESPONSE_HEADER_SIZE, (UINT32*)FtpmBuffer);

  ///
  /// Check the reponse data header (tag, parasize and returncode)
  ///
  CopyMem (&Data16, FtpmBuffer, sizeof (UINT16));
  DEBUG ((EFI_D_INFO, "TPM2_RESPONSE_HEADER.tag = 0x%04x\n", SwapBytes16(Data16)));

  ///
  /// TPM Rev 2.0 Part 2 - 6.9 TPM_ST (Structure Tags)
  /// TPM_ST_RSP_COMMAND - Used in a response that has an error in the tag.
  ///
  if (SwapBytes16(Data16) == TPM_ST_RSP_COMMAND) {
    DEBUG ((EFI_D_ERROR, "TPM2_RESPONSE_HEADER.tag = TPM_ST_RSP_COMMAND - Error in response!\n"));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  CopyMem(&Data32, (FtpmBuffer + 2), sizeof(UINT32));
  DEBUG ((EFI_D_INFO, "TPM2_RESPONSE_HEADER.paramSize = 0x%08x\n", SwapBytes32(Data32)));

  DataLength = SwapBytes32(Data32);

  if(DataLength < sizeof(TPM2_RESPONSE_HEADER)) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  if (*RespSize < DataLength) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }

  if(DataLength > S_PTT_HCI_CRB_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }

  *RespSize = DataLength;
  ///
  /// Align command size to dword before writing to FTPM_CRB
  ///
  if(DataLength % 4 != 0) {
    DEBUG ((EFI_D_INFO, "Alignment: RespSize change from %d ", DataLength));
    DataLength += (4 - (DataLength % 4));
    DEBUG ((EFI_D_INFO, "to %d\n", DataLength));
  }

  ///
  /// Reading the entire response data
  ///
  MmioReadBuffer32((UINTN)mPttBaseAddress + R_PTT_HCI_CRB, DataLength, (UINT32*)FtpmBuffer);

Exit:
  if(!EFI_ERROR(Status)) {
    ///
    /// FTPM_CA_CMD - the physical address from which the TPM 2.0 driver will read command responses
    ///
    /*      MmioWrite32((UINTN)(mPttBaseAddress + R_PTT_HCI_CA_RSP), mPttBaseAddress + R_PTT_HCI_CRB);
          MmioWrite32((UINTN)(mPttBaseAddress + R_PTT_HCI_CA_RSP_SZ), *RespSize);
          DEBUG ((EFI_D_INFO, "FTPM_CA_RSP (@ 0x%08x) written, value = 0x%08x\n",
                              (mPttBaseAddress + R_PTT_HCI_CA_RSP),
                              (mPttBaseAddress + R_PTT_HCI_CRB)));
          DEBUG ((EFI_D_INFO, "FTPM_CA_RSP_SZ (@ 0x%08x) written, value = 0x%08x\n",
                              (mPttBaseAddress + R_PTT_HCI_CA_RSP_SZ), *RespSize));*/
  }

  return Status;
}


VOID RecordPerf(
  UINT8    *Data,
  BOOLEAN  IsStart
  )
{
  CHAR8   CommandName[] = "CMD0000";
  CommandName[3] = ((Data[8] >> 4) <= 9) ? ((Data[8] >> 4) + '0') : ((Data[8] >> 4) - 9 + 'A');
  CommandName[4] = ((Data[8] & 0xf) <= 9) ? ((Data[8] & 0xf) + '0') : ((Data[8] & 0xf ) - 9 + 'A');
  CommandName[5] = ((Data[9] >> 4) <= 9) ? ((Data[9] >> 4) + '0') : ((Data[9] >> 4) - 9 + 'A');
  CommandName[6] = ((Data[9] & 0xf) <= 9) ? ((Data[9] & 0xf) + '0') : ((Data[9] & 0xf ) - 9 + 'A');
  if (IsStart) {
    PERF_START (NULL, CommandName, "FTPM", 0);
  } else {
    PERF_END (NULL, CommandName, "FTPM", 0);
  }
}




/**
  This service enables the sending of commands to the TPM2.

  @param[in]  InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]  InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]  OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small.
**/
EFI_STATUS
EFIAPI
Tpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  EFI_STATUS Status;
  DEBUG ((EFI_D_INFO, "PTT: PttHciSubmitCommand start\n"));

  if(InputParameterBlock == NULL || OutputParameterBlock == NULL || InputParameterBlockSize == 0) {
    DEBUG ((EFI_D_ERROR, "Buffer == NULL or InputParameterBlockSize == 0\n"));
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (PcdGetBool(PcdFTPMNotRespond)) {
    Status = EFI_NOT_READY;
    goto Done;
  }

  RecordPerf(InputParameterBlock, TRUE);
  DEBUG ((EFI_D_INFO, "PTT: Command Buffer dump\n"));

  DEBUG_CODE(PttHciPrintBuffer(InputParameterBlock, InputParameterBlockSize););

  ///
  /// Send the command to TPM
  ///
  Status = PttHciSend(InputParameterBlock, InputParameterBlockSize);
  if (EFI_ERROR (Status))  {
    DEBUG ((EFI_D_ERROR, "FTpmHciSend EFI_ERROR = %r\n", Status));
    goto Done;
  }

  ///
  /// Receive the response data from TPM
  ///
  Status = PttHciReceive(OutputParameterBlock, OutputParameterBlockSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "FTpmHciReceive EFI_ERROR = %r\n", Status));
    goto Done;
  }

  DEBUG ((EFI_D_INFO, "PTT: Response Buffer dump\n"););


Done:
  if ((Status == EFI_TIMEOUT) || (Status == EFI_NOT_READY)) {
    PcdSetBool(PcdFTPMNotRespond, TRUE);
  }
  if (InputParameterBlock != NULL) {
    RecordPerf(InputParameterBlock, FALSE);
  }

  return Status;
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2RequestUseTpm (
  VOID
  )
{
  if (!(PcdGetBool (PcdMeasuredBootEnable))) {
    DEBUG ((EFI_D_ERROR, "Measured Boot Disable\n"));
    return EFI_NOT_FOUND;
  }
  DEBUG ((EFI_D_ERROR, "Measured Boot Enable\n"));
  if (mPttBaseAddress == 0) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

/**
  This service register TPM2 device.

  @Param Tpm2Device  TPM2 device

  @retval EFI_SUCCESS          This TPM2 device is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this TPM2 device.
  @retval EFI_ALREADY_STARTED  System already register this TPM2 device.
**/
EFI_STATUS
EFIAPI
Tpm2RegisterTpm2DeviceLib (
  IN TPM2_DEVICE_INTERFACE   *Tpm2Device
  )
{
  return EFI_UNSUPPORTED;
}


