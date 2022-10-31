/** @file
  Implement the I2C port control

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/I2cPort_platform.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "I2cRegs_A0.h"
#include "I2cPortDxe.h"
#include "I2cPort.h"
//#pragma optimize ("",off)

extern void local_irq_save( UINTN * eflags);
extern void local_irq_restore( UINTN eflags);
extern UINT64 EfiReadTsc();


///
/// Context maintained between configuration and start routines
///
typedef struct {
  UINT16 I2cMode;         ///<  Controller mode
} I2C_A0_PIO_PORT_CONTEXT;


///
/// Port driver context size
///
CONST UINTN mI2cContextLengthInBytes = sizeof ( I2C_A0_PIO_PORT_CONTEXT );
UINT64 rdtsc_frequency = 0;   //rdtsc frequency, in HZ, calibrated during I2cReset


/**
  Computes CPU rdtsc frequenc

  @Frequency returns the calibrated rdtsc frequency in MHZ
  @retval RETURN_SUCCESS      The operation was successful
  @retval EFI_INVALID_PARAMETER

**/



EFI_STATUS
CalibrateTscFrequency (
  OUT UINT64                        *Frequency
  )
{
  UINT64  BeginValue;
  UINT64  EndValue;

  if (Frequency == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Read timestamp counter before and after delay of 100 microseconds
  //
  BeginValue = EfiReadTsc ();
  MicroSecondDelay (100);
  EndValue   = EfiReadTsc ();
  //
  // Calculate the actual frequency
  //
// *Frequency  = DivU64x32Remainder (EndValue - BeginValue, 100, NULL);
// *Frequency  = DivU64x32Remainder (EndValue - BeginValue, 100, NULL) * 1000000;
  *Frequency  = MultU64x32(EndValue - BeginValue, 10000);
  DEBUG((EFI_D_INFO, "RDTSC frequency: %lld HZ\r\n", *Frequency));

  return EFI_SUCCESS;
}




/**
  Determine the state of the I2C controller

  @param[in] PlatformData   Address of the platform configuration data

  @retval TRUE              The I2C controller is active
  @retval FALSE             The I2C controller is idle

**/
BOOLEAN
I2cIsHardwareActive (
  IN CONST VOID *PlatformData
  )
{
  UINTN BaseAddr;
  CONST I2C_PIO_PLATFORM_CONTEXT *PlatformContext;
  PlatformContext = (I2C_PIO_PLATFORM_CONTEXT *)PlatformData;
  BaseAddr = (UINTN)PlatformContext->BaseAddress;
  if ( 0 == ( MmioRead32 ( BaseAddr + R_IC_STATUS) & STAT_MST_ACTIVITY )) {
    return FALSE;
  }
  return TRUE;
}


/**
  Compute the counter values for the I2C controller

  @param[in] InputClockHertz  Input clock frequency in Hertz
  @param[in] BusClockHertz    New I2C bus clock frequency in Hertz
  @param[in] MaxHighValue     High counter upper limitation
  @param[in] MinHighValue     High counter lower limitation
  @param[out] HighValue       Address to receive high count value
  @param[in] MaxLowValue      Low counter upper limitation
  @param[in] MinLowValue      Low counter lower limitation
  @param[out] LowValue        Address to receive low count value

  @retval RETURN_SUCCESS      The operation was successful
  @retval RETURN_UNSUPPORTED  The selected speed is not supported

**/
RETURN_STATUS
I2cClockValues (
  IN  UINTN InputClockHertz,
  IN  UINTN BusClockHertz,
  IN  UINTN MaxHighValue,
  IN  UINTN MinHighValue,
  OUT UINTN *HighValue,
  IN  UINTN MaxLowValue,
  IN  UINTN MinLowValue,
  OUT UINTN *LowValue
  )
{
  UINTN High;
  UINTN Low;
  EFI_STATUS Status;
  UINTN Value;

  //
  //  Assume success
  //
  Status = RETURN_SUCCESS;

  //
  //  Determine the total count
  //
  Value = ( InputClockHertz + BusClockHertz - 1 ) / BusClockHertz;
  High = Value >> 1;
  Low = Value - High;

  //
  //  Adjust for the counter behavior
  //
  High -= 8 + 15;
  Low  -= 1 + 15;
  DEBUG((EFI_D_INFO, "I2cClockValues adjusted High: 0x%x Low: 0x%x\r\n", High, Low));

  //
  //  Determine if this is a valid clock frequency
  //
  if (( MaxHighValue < High ) || ( MaxLowValue < Low )) {
    //
    //  The clock frequency is too low
    //
    Status = RETURN_UNSUPPORTED;
  }

  //
  //  Stretch the value if necessary
  //
  if ( MinHighValue > High ) {
    High = MinHighValue;
  }
  if ( MinLowValue > Low ) {
    Low = MinLowValue;
  }

  //
  //  Return the values
  //
  *HighValue = High;
  *LowValue = Low;
  return Status;
}


/**
  Disable and Enable the I2C controller

  @param[in] PlatformContext  Address of the platform configuration data

  @retval RETURN_SUCCESS    The operation was successful
  @retval RETURN_NOT_READY  The operation fails in given time duration

**/
RETURN_STATUS
I2cDisable (
  IN CONST I2C_PIO_PLATFORM_CONTEXT *PlatformContext
  )
{
  UINTN BaseAddr;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  BaseAddr = (UINTN)PlatformContext->BaseAddress;
  MmioWrite16 ( BaseAddr + R_IC_ENABLE, 0 );
  while ( 0 != ( MmioRead16 ( BaseAddr + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}

RETURN_STATUS
I2cEnable (
  IN CONST I2C_PIO_PLATFORM_CONTEXT *PlatformContext
  )
{
  UINTN BaseAddr;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  BaseAddr = (UINTN)PlatformContext->BaseAddress;
  MmioWrite16 ( BaseAddr + R_IC_ENABLE, 1 );
  while ( 0 == ( MmioRead16 ( BaseAddr + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}




/**
  Set the I2C controller bus clock frequency.

  @param[in] This           Address of the library's I2C context structure
  @param[in] PlatformData   Address of the platform configuration data
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval RETURN_SUCCESS      The bus frequency was set successfully.
  @retval RETURN_UNSUPPORTED  The controller does not support this frequency.

**/
RETURN_STATUS
I2cBusFrequencySet (
  IN VOID *This,
  IN CONST VOID *PlatformData,
  IN UINTN BusClockHertz
  )
{
  UINTN BaseAddr;
//  UINTN High;
  UINTN I2cMode;
//  UINTN Low;
  I2C_PIO_PLATFORM_CONTEXT *PlatformContext;
  I2C_A0_PIO_PORT_CONTEXT *PortContext;
  RETURN_STATUS Status;


  //
  //  Assume success
  //
  Status = EFI_SUCCESS;
  PortContext = (I2C_A0_PIO_PORT_CONTEXT *)This;
  PlatformContext = (I2C_PIO_PLATFORM_CONTEXT *)PlatformData;

  DEBUG((EFI_D_INFO, "I2cBusFrequencySet entered\r\n" ));
  DEBUG((EFI_D_INFO, "  InputFreq BusClockHertz: %d %d\r\n",PlatformContext->InputFrequencyHertz, BusClockHertz));
  Status = I2cDisable(PlatformContext);
  DEBUG((EFI_D_INFO, "I2cDisable Status %r\r\n", Status));
  BaseAddr = (UINTN)PlatformContext->BaseAddress;
  I2cMode = 0;
  switch ( BusClockHertz ) {
    case 100 * 1000:
      I2cMode = V_SPEED_STANDARD;
      MmioWrite32 ( BaseAddr + R_IC_SDA_HOLD,    (UINT16)0x40);
      break;

    case 400 * 1000:
      I2cMode = V_SPEED_FAST;
      MmioWrite32 ( BaseAddr + R_IC_SDA_HOLD,    (UINT16)0x32);
      break;

    case 3400 * 1000:
      I2cMode = V_SPEED_HIGH;
      MmioWrite32 ( BaseAddr + R_IC_SDA_HOLD,    (UINT16)0x9);
      //
      //  Set the 1000 KHz clock divider
      //
      //  From Table 10 of the I2C specification
      //
      //    High: 0.26 uS
      //    Low:  0.50 uS
      //
      //  Set the 3400 KHz clock divider
      //
      //  From Table 12 of the I2C specification
      //
      //    High: 0.06 uS
      //    Low:  0.16 uS
      //
      //  Set the 5000 KHz clock divider
      //
      //  From Table 14 of the I2C specification
      //
      //    High: 0.05 uS
      //    Low:  0.05 uS
      //
      //  Compute the clock values
      //
      /*
            Status = I2cClockValues ( PlatformContext->InputFrequencyHertz,
                                      BusClockHertz,
                                      65525,
                                      6,
                                      &High,
                                      65535,
                                      8,
                                      &Low );
            ASSERT ( RETURN_SUCCESS == Status );

            //
            //  Set the clock frequency for high speed
            //
            MmioWrite16 ( BaseAddr + R_IC_HS_SCL_HCNT, (UINT16)High );
            MmioWrite16 ( BaseAddr + R_IC_HS_SCL_LCNT, (UINT16)Low );
      */
      break;
  }
  if ( !EFI_ERROR ( Status )) {
    //
    //  Select the frequency counter
    //  Enable restart condition,
    //  Enable master FSM, disable slave FSM
    //
    I2cMode |= B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;
    PortContext->I2cMode = (UINT16)I2cMode;
  }


  //
  //  Return the configuration status
  //

  DEBUG(( EFI_D_INFO, "I2cBusFrequencySet exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Reset the I2C controller and configure it for use

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in] This           Address of the library context structure
  @param[in] PlatformData   Address of the platform configuration data

**/
VOID
I2cReset (
  IN VOID *This,
  IN CONST VOID *PlatformData
  )
{
  UINTN BaseAddr;
  UINTN BusFrequencyHertz;
//  UINTN High;
//  UINTN Low;
  I2C_PIO_PLATFORM_CONTEXT *PlatformContext;
  I2C_A0_PIO_PORT_CONTEXT *PortContext;
  UINTN I2cMode;
  EFI_STATUS Status;


  PlatformContext = (I2C_PIO_PLATFORM_CONTEXT *)PlatformData;
  PortContext = (I2C_A0_PIO_PORT_CONTEXT *) This;
  BaseAddr = (UINTN)PlatformContext->BaseAddress;

  if(rdtsc_frequency == 0) {
    CalibrateTscFrequency(&rdtsc_frequency);
  }

  //
  // Release Resets
  //
  MmioWrite32 (
    BaseAddr + 0x804,
    0x3
  );
  //
  // Activate Clocks
  //
  MmioWrite32 (
    BaseAddr + 0x800,
    0x80010003
  );

  DEBUG((EFI_D_INFO, "I2cReset\r\n"));
  Status = I2cDisable ( PlatformContext );
  DEBUG((EFI_D_INFO, "I2cDisable Status = %r\r\n", Status));

  //
  //  Set the 100 KHz clock divider
  //
  //  From Table 10 of the I2C specification
  //
  //    High: 4.00 uS
  //    Low:  4.70 uS
  //

  PlatformContext = (I2C_PIO_PLATFORM_CONTEXT *)PlatformData;

  {
    BusFrequencyHertz = 100 * 1000;
    Status = RETURN_SUCCESS;
    /*
        Status = I2cClockValues ( PlatformContext->InputFrequencyHertz,
                                  BusFrequencyHertz,
                                  65525,
                                  6,
                                  &High,
                                  65535,
                                  8,
                                  &Low );

        ASSERT ( RETURN_SUCCESS == Status );
        DEBUG((EFI_D_INFO, "100khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));
        MmioWrite16 ( BaseAddr + R_IC_SS_SCL_HCNT, (UINT16)High );
        MmioWrite16 ( BaseAddr + R_IC_SS_SCL_LCNT, (UINT16)Low );
    */
    MmioWrite16 ( BaseAddr + R_IC_SS_SCL_HCNT, (UINT16)0x214 );
    MmioWrite16 ( BaseAddr + R_IC_SS_SCL_LCNT, (UINT16)0x272 );
    MmioWrite32 ( BaseAddr + R_IC_SDA_HOLD,    (UINT16)0x40);

    //
    //  Set the 400 KHz clock divider
    //
    //  From Table 10 of the I2C specification
    //
    //    High: 0.60 uS
    //    Low:  1.30 uS
    //
    BusFrequencyHertz = 400 * 1000;
    /*    Status = I2cClockValues ( PlatformContext->InputFrequencyHertz,
                                  BusFrequencyHertz,
                                  65525,
                                  6,
                                  &High,
                                  65535,
                                  8,
                                  &Low );
        ASSERT ( RETURN_SUCCESS == Status );
        DEBUG((EFI_D_INFO, "400khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));
        MmioWrite16 ( BaseAddr + R_IC_FS_SCL_HCNT, (UINT16)High );
        MmioWrite16 ( BaseAddr + R_IC_FS_SCL_LCNT, (UINT16)Low );
    */
    MmioWrite16 ( BaseAddr + R_IC_FS_SCL_HCNT, (UINT16)0x50 );
    MmioWrite16 ( BaseAddr + R_IC_FS_SCL_LCNT, (UINT16)0xAD );

    BusFrequencyHertz = 3400 * 1000;
    MmioWrite16 ( BaseAddr + R_IC_HS_SCL_HCNT, (UINT16)0xB);
    MmioWrite16 ( BaseAddr + R_IC_HS_SCL_LCNT, (UINT16)0x13);
  }


  //
  //  Set the default clock speed of 100 KHz
  //
  BusFrequencyHertz = 100 *1000;
  MmioWrite32 ( BaseAddr + R_IC_SDA_HOLD,    (UINT16)0x40);
  I2cMode = B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;
  I2cMode |= V_SPEED_STANDARD;
  PortContext->I2cMode = (UINT16)I2cMode;

  Status = I2cEnable ( PlatformContext );
  DEBUG((EFI_D_INFO, "I2cEnable Status = %r\r\n", Status));
}


/**f
  Start an I2C operation on the controller

  This function initiates an I2C operation on the controller.

  N.B. This API supports only one operation, no queuing support
  exists at this layer.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all write data to the I2C device.  If read data
  is requested, a restart is sent followed by the slave address and then
  the read data is clocked into the I2C controller and placed in the read
  buffer.  When the operation completes, the status value is returned and
  then the event is set.

  @param[in]  This          Address of the library context structure
  @param[in]  PlatformData  Address of the platform configuration data
  @param[in]  SlaveAddress  Address of the device on the I2C bus.
  @param[in]  WriteBytes    Number of bytes to send
  @param[in]  WriteBuffer   Address of buffer containing data to send
  @param[in]  ReadBytes     Number of bytes to read
  @param[out] ReadBuffer    Address of buffer to receive data

  @retval RETURN_SUCCESS            The operation completed successfully.
  @retval RETURN_DEVICE_ERROR       There was an I2C error (NACK) during the operation.
                                    This could indicate the slave device is not present.
  @retval RETURN_INVALID_PARAMETER  NULL specified for pConfig
  @retval RETURN_NOT_FOUND          SlaveAddress exceeds maximum address
  @retval RETURN_NOT_READY          I2C bus is busy or operation pending, wait for
                                    the event and then read status.
  @retval RETURN_NO_RESPONSE        The I2C device is not responding to the
                                    slave address.  EFI_DEVICE_ERROR may also be
                                    returned if the controller can not distinguish
                                    when the NACK occurred.
  @retval RETURN_OUT_OF_RESOURCES   Insufficient memory for I2C operation
  @retval RETURN_TIMEOUT            The transaction did not complete within an internally
                                    specified timeout period.

**/
RETURN_STATUS
I2cStartRequest (
  IN  VOID *This,
  IN  CONST VOID *PlatformData,
  IN  UINTN SlaveAddress,
  IN  UINTN WriteBytes,
  IN  UINT8 *WriteBuffer,
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer,
  IN  UINT32 Timeout
  )
{
  UINTN BaseAddr;
  UINT32 I2cStatus;
  I2C_PIO_PLATFORM_CONTEXT *PlatformContext;
  I2C_A0_PIO_PORT_CONTEXT *PortContext;
  UINT16 ReceiveData;
  UINT8 *ReceiveDataEnd ;
  UINT8 *ReceiveRequest = NULL;
  RETURN_STATUS Status;
  UINT8 *TransmitEnd;
  UINT16 raw_intr_stat = 0;
  UINT32 NumTries = 0;
  UINTN eflags;
  UINT64 rdtsc_start, rdtsc_end, rdtsc_interval;
  DEBUG((EFI_D_INFO,"I2cStartRequest Entered\r\n"));


  //
  //  Verify the parameters
  //
  Status = RETURN_SUCCESS;
  PortContext = (I2C_A0_PIO_PORT_CONTEXT *)This;
  PlatformContext = (I2C_PIO_PLATFORM_CONTEXT *)PlatformData;

  if (( NULL == PlatformData ) || ( 1023 < SlaveAddress )) {
    Status =  RETURN_INVALID_PARAMETER;
    goto _exit_I2cStartRequest;
  }


  BaseAddr = (UINTN)PlatformContext->BaseAddress;
  DEBUG((EFI_D_INFO, "Base address 0x%x\r\n",BaseAddr));
  NumTries = 100 * 1000;   /* 1 seconds */
  while ( I2cIsHardwareActive ( PlatformData )) {
    MicroSecondDelay(10);
    NumTries --;
    if(0 == NumTries) {
      Status = RETURN_DEVICE_ERROR;
      goto _exit_I2cStartRequest;
    }
  }
  DEBUG((EFI_D_INFO, "I2c hardware is inactive now\r\n"));
  Status = I2cDisable ( PlatformContext );
  DEBUG((EFI_D_INFO, "I2cDisable Status = %r\r\n", Status));
  MmioWrite16( BaseAddr + R_IC_CLK_GATE, 0x01);
  MmioWrite16( BaseAddr + R_IC_INTR_MASK, 0x0);
  if ( 0x7f < SlaveAddress ) {
    SlaveAddress = ( SlaveAddress & 0x3ff )
                   | IC_TAR_10BITADDR_MASTER;
  }
  MmioWrite16 ( BaseAddr + R_IC_TAR, (UINT16) SlaveAddress );
  MmioWrite16 ( BaseAddr + R_IC_RX_TL, 0);
  MmioWrite16 ( BaseAddr + R_IC_TX_TL, 0 );
  MmioWrite16 ( BaseAddr + R_IC_CON, PortContext->I2cMode );
  Status = I2cEnable(PlatformContext);
  DEBUG((EFI_D_INFO, "I2cEnable Status = %r\r\n", Status));
  if( EFI_ERROR(Status) ) {
    Status = RETURN_DEVICE_ERROR;
    goto _exit_I2cStartRequest;
  }
  MmioRead16 ( BaseAddr + R_IC_CLR_TX_ABRT );

  ReceiveDataEnd = &ReadBuffer [ ReadBytes ];
  TransmitEnd = &WriteBuffer [ WriteBytes ];
  rdtsc_start = EfiReadTsc();
  rdtsc_end = rdtsc_start + MultU64x32(DivU64x32Remainder(rdtsc_frequency, 1000, NULL), Timeout) ;
  DEBUG((EFI_D_INFO, "rdtsc_start:     %lld\n", rdtsc_start));
  DEBUG((EFI_D_INFO, "rdtsc_timeout:   %lld\n", rdtsc_end));
  if(rdtsc_end < rdtsc_start) {
    DEBUG((EFI_D_ERROR, "Rdtsc overflow\n"));
    Status = RETURN_INVALID_PARAMETER;
    goto _exit_I2cStartRequest;
  }

  Status = EFI_SUCCESS;
  if( WriteBytes ) {
    DEBUG((EFI_D_INFO,
        "Write: Enters inner loop TransmitEnd: 0x%016Lx WriteBuffer:0x%016Lx %d bytes to be transfered\r\n", 
        (UINT64)(UINTN)TransmitEnd, 
        (UINT64)(UINTN)WriteBuffer,
        TransmitEnd - WriteBuffer));
    local_irq_save(&eflags);
    while ( TransmitEnd > WriteBuffer ) {
      I2cStatus = MmioRead16 ( BaseAddr + R_IC_STATUS );
      raw_intr_stat = MmioRead16 ( BaseAddr + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead16 ( BaseAddr + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_ERROR, "TX ABRT TransmitEnd:0x%016Lx WriteBuffer:0x%016Lx\r\n", (UINT64)(UINTN)TransmitEnd, (UINT64)(UINTN)WriteBuffer));
        break;
      }
      if ( EfiReadTsc() > rdtsc_end ) {
        //
        //  This shouldn't happen, the timeout value should be enough to transfer all the bytes.
        //  If timeout value isn't enough, do the following workaround
        //
        rdtsc_end += 1000000;
        while( (0 == (MmioRead16 ( BaseAddr + R_IC_STATUS ) & STAT_TFNF) ) &&
               ( EfiReadTsc() < rdtsc_end )
               );
        if( MmioRead16 ( BaseAddr + R_IC_STATUS ) & STAT_TFNF ) {
          MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, *WriteBuffer | B_CMD_STOP );
          MicroSecondDelay(2000);  //finish the data transmission in fifo
        } else {
          DEBUG((EFI_D_INFO, "Tricky things happen!!!\r\n\r\n\r\n"));
        }
        Status = RETURN_TIMEOUT;
        DEBUG((EFI_D_ERROR, "TX timeout TransmitEnd:0x%016Lx WriteBuffer:0x%016Lx\r\n", (UINT64)(UINTN)TransmitEnd, (UINT64)(UINTN)(++WriteBuffer)));
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        continue;
      }
      if( TransmitEnd == (WriteBuffer + 1) ) {
        //
        //  For WRITE only transaction, write B_CMD_STOP to issue bus stop signal
        //  For WRITE-READ sequence, READ loop will issue bus stop signal
        //  VLV A0/B0 I2C MASTER controller will stretch SCL to low without B_CMD_STOP signal.
        //
        if( ReadBytes == 0 ) {
          MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, *WriteBuffer | B_CMD_STOP );
        } else {
          MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, *WriteBuffer );
        }
        WriteBuffer ++;
      } else {
        MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, *WriteBuffer++ );
      }
      //
      //  Add a small delay to work around some odd behavior being seen.  Without
      //  this delay bytes get dropped.
      //
      MicroSecondDelay ( FIFO_WRITE_DELAY );
    }
    local_irq_restore(eflags);

    if( EFI_ERROR(Status) ) goto _exit_I2cStartRequest;
    //
    //  If this transaction is WRITE only, rather than WRITE-READ sequence,wait for bytes to go
    //  Write into Tx fifo doesn't mean the dat will go correctly on the SDA data line
    //
    if( ReadBytes == 0) {
      while( 1 ) {
        raw_intr_stat = MmioRead16 ( BaseAddr + R_IC_RAW_INTR_STAT );
        if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
          MmioRead16 ( BaseAddr + R_IC_CLR_TX_ABRT );
          Status = RETURN_DEVICE_ERROR;
          DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
          goto _exit_I2cStartRequest;
        }
        if( 0 == MmioRead16(BaseAddr + R_IC_TXFLR ) ) break;
        if( EfiReadTsc() > rdtsc_end ) {
          Status = RETURN_TIMEOUT;
          DEBUG((EFI_D_ERROR, "TX timeout TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
          goto _exit_I2cStartRequest;
        }
      }
    }
  }
  if(EFI_ERROR(Status)) goto _exit_I2cStartRequest;

  if( ReadBytes ) {
    ReceiveRequest = ReadBuffer;
    DEBUG((EFI_D_INFO,
        "Read: Enters inner loop ReceiveDataEnd:0x%016Lx ReceiveRequest:0x%016Lx %d bytes to be transfered\r\n", 
        (UINT64)(UINTN)ReceiveDataEnd, 
        (UINT64)(UINTN)ReceiveRequest,
        ReceiveDataEnd - ReceiveRequest));
    local_irq_save(&eflags);
    while ( (ReceiveDataEnd > ReceiveRequest) ||
            (ReceiveDataEnd > ReadBuffer)) {
      raw_intr_stat = MmioRead16 ( BaseAddr + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead16 ( BaseAddr + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_INFO,"TX ABRT ,%d bytes hasn't been transferred\r\n", ReceiveDataEnd - ReceiveRequest));
        break;
      }
      //
      //  Determine if another byte was received
      //
      I2cStatus = MmioRead16 ( BaseAddr + R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = MmioRead16 ( BaseAddr + R_IC_DATA_CMD );
        *ReadBuffer++ = (UINT8)ReceiveData;
      }

      if ( EfiReadTsc() > rdtsc_end ) {
        //
        //  This shouldn't happen , the timeout value should be enough to transfer all the bytes.
        //  If timeout value isn't enough, do the following workaround
        //
        if( ReceiveDataEnd > ReceiveRequest ) {
          rdtsc_end += 1000000;
          while( (0 == (MmioRead16 ( BaseAddr + R_IC_STATUS ) & STAT_TFNF) ) &&
                 ( EfiReadTsc() < rdtsc_end )
                 );
          if( MmioRead16 ( BaseAddr + R_IC_STATUS ) & STAT_TFNF ) {
            MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, B_READ_CMD | B_CMD_STOP );
            MicroSecondDelay(2000);  //finish the data transmission in fifo
          } else {
            DEBUG((EFI_D_INFO, "Tricky things happen!!!\r\n\r\n\r\n"));
          }
        } else {
          DEBUG((EFI_D_INFO, "All read command has been transferred\r\n"));
        }
        Status = RETURN_TIMEOUT;
        DEBUG((EFI_D_INFO, "RX timeout ,%d bytes hasn't been transferred\r\n",ReceiveDataEnd - ReceiveRequest));
        break;
      }

      if(ReceiveDataEnd == ReceiveRequest) continue;
      //
      //  Wait until a read request will fit
      //
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        MicroSecondDelay ( 10 );
        continue;
      }
      //
      //  Issue the next read request
      //
      if(ReceiveDataEnd == ( ReceiveRequest + 1 ) ) {
        MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, B_READ_CMD | B_CMD_STOP );
      } else {
        MmioWrite16 ( BaseAddr + R_IC_DATA_CMD, B_READ_CMD );
      }
      ReceiveRequest += 1;
    }
    local_irq_restore(eflags);
  }

  rdtsc_interval = EfiReadTsc() - rdtsc_start;
  DEBUG((EFI_D_INFO, "Duration: %d us\r\n", DivU64x32Remainder (rdtsc_interval, (UINT32)DivU64x32Remainder(rdtsc_frequency, 1000000, NULL), NULL)));
_exit_I2cStartRequest:
  DEBUG((EFI_D_INFO, "I2cStartRequest Exit with Status %r\r\n",Status));
//  Status = I2cDisable ( PlatformContext );
  return Status;
}
