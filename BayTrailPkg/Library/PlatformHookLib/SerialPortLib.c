/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation
  (if any) is furnished under a license and may only be used or
  copied in accordance with the terms of the license.  Except as
  permitted by such license, no part of this software or
  documentation may be reproduced, stored in a retrieval system, or
  transmitted in any form or by any means without the express written
  consent of Intel Corporation.

  Module Name:  SerialPortLib.c

**/

#include "PlatformSerialPortLib.h"

#include "PchRegs/PchRegsPcu.h"
#include "PlatformBaseAddresses.h"
#include "token.h"
#include <Library/PchPlatformLib.h>

#define PCI_IDX        0xCF8
#define PCI_DAT        0xCFC
#define PCI_LPC_BASE    (0x8000F800)
#define PCI_LPC_REG(x)  (PCI_LPC_BASE + (x))
//#define PCIEX_BASE_ADDRESS                        0xE0000000
#define PciD31F0RegBase                           PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)
#define V_PCH_ILB_IRQE_UARTIRQEN_IRQ3             BIT3 // UART IRQ3 Enable
#define V_PCH_ILB_IRQE_UARTIRQEN_IRQ4             BIT4 // UART IRQ4 Enable

UINT16 gComBase  = 0x3f8;
UINTN  gBps      = 115200;
UINT8  gData     = 8;
UINT8  gStop     = 1;
UINT8  gParity   = 0;
UINT8  gBreakSet = 0;

RETURN_STATUS
EFIAPI
UARTInitialize (
  VOID
  )
/*++

Routine Description:

  Initialize Serial Port

    The Baud Rate Divisor registers are programmed and the LCR
    is used to configure the communications format. Hard coded
    UART config comes from globals in DebugSerialPlatform lib.

Arguments:

  None

Returns:

  None

--*/
{
  UINTN Divisor;
  UINT8 OutputData;
  UINT8 Data;

  //
  // Map 5..8 to 0..3
  //
  Data = (UINT8) (gData - (UINT8) 5);

  //
  // Calculate divisor for baud generator
  //
  Divisor = 115200 / gBps;

  //
  // Set communications format
  //
  OutputData = (UINT8) ((DLAB << 7) | ((gBreakSet << 6) | ((gParity << 3) | ((gStop << 2) | Data))));
  IoWrite8 (gComBase + LCR_OFFSET, OutputData);

  //
  // Configure baud rate
  //
  IoWrite8 (gComBase + BAUD_HIGH_OFFSET, (UINT8) (Divisor >> 8));
  IoWrite8 (gComBase + BAUD_LOW_OFFSET, (UINT8) (Divisor & 0xff));

  //
  // Switch back to bank 0
  //
  OutputData = (UINT8) ((~DLAB << 7) | ((gBreakSet << 6) | ((gParity << 3) | ((gStop << 2) | Data))));
  IoWrite8 (gComBase + LCR_OFFSET, OutputData);

  return RETURN_SUCCESS;
}

#if SERIAL_DEBUGGER_SUPPORT == 0
RETURN_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
/*++

Routine Description:

  Common function to initialize UART Serial device and USB Serial device.

Arguments:

  None

Returns:

  None

--*/
{
//
//  UINT32 Data32; //EIP134867
//  
//  // Force enabled UART decode start.
//  // Program and enable PMC Base.
//  IoWrite32 (PCI_IDX,  PCI_LPC_REG(R_PCH_LPC_PMC_BASE));
//  IoWrite32 (PCI_DAT,  (PMC_BASE_ADDRESS | B_PCH_LPC_PMC_BASE_EN));
//
//  // Enable COM1 for debug message output.
//  //EIP134867 >>
//  Data32 = MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
//  // Do not clear SUS Well Power Failure, General Reset Status and RTC Power Status bits
//  //EIP139043 >>
//  // BIOS should clear RTC_PWR_STS bit (PBASE + 0x20[2]) by writing a '0b' to this bit position
////  Data32 &= (UINT32) ~(B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS | B_PCH_PMC_GEN_PMCON_RTC_PWR_STS);
//  Data32 &= (UINT32) ~(B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS);
//  //EIP139043 <<
//  Data32 |= BIT24;
//  MmioWrite32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Data32);
//  //EIP134867 <<
//
//  if (PchStepping()>= PchB0)
//    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) V_PCH_ILB_IRQE_UARTIRQEN_IRQ4);
//  else
//    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) V_PCH_ILB_IRQE_UARTIRQEN_IRQ3);
//  MmioAnd32(IO_BASE_ADDRESS + 0x0520, (UINT32)~(0x00000187));
//  MmioOr32 (IO_BASE_ADDRESS + 0x0520, (UINT32)0x81); // UART3_RXD-L
//  MmioAnd32(IO_BASE_ADDRESS + 0x0530, (UINT32)~(0x00000007));
//  MmioOr32 (IO_BASE_ADDRESS + 0x0530, (UINT32)0x1); // UART3_RXD-L
//  MmioOr8 (PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) B_PCH_LPC_UART_CTRL_COM1_EN);
//  // Force enabled UART decode end.

//  InitializeSio ();

  UARTInitialize ();

  //-if (FeaturePcdGet (PcdStatusCodeUseUsbSerial)) {
  //-  UsbDebugPortInitialize ( );
  //-}

  return RETURN_SUCCESS;
}
#endif 

/**
  Write data to serial device.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes writed to serial device.

**/
UINTN
EFIAPI
UARTDbgOut (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{
  UINTN Result;
  UINT8 Data;

  if (NULL == Buffer) {
    return 0;
  }

  Result = NumberOfBytes;

  while (NumberOfBytes--) {
    //
    // Wait for the serial port to be ready.
    //
    do {
      Data = IoRead8 ((UINT16) PcdGet64 (PcdSerialRegisterBase) + LSR_OFFSET);
    } while ((Data & LSR_TXRDY) == 0);
    IoWrite8 ((UINT16) PcdGet64 (PcdSerialRegisterBase), *Buffer++);
  }

  return Result;
}

#if SERIAL_DEBUGGER_SUPPORT == 0
/**
  Common function to write data to UART Serial device and USB Serial device.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
)
{

  if (FeaturePcdGet (PcdStatusCodeUseIsaSerial)) {
    UARTDbgOut (Buffer, NumberOfBytes);
  }

  //-if (FeaturePcdGet (PcdStatusCodeUseUsbSerial)) {
  //-  UsbDebugPortWrite (Buffer, NumberOfBytes);
  //-}

  return RETURN_SUCCESS;
}
#endif

/*
  Read data from serial device and save the datas in buffer.

  If the buffer is NULL, then return 0;
  if NumberOfBytes is zero, then return 0.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Actual number of bytes raed to serial device.

**/
UINTN
EFIAPI
UARTDbgIn (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  UINTN Result;
  UINT8 Data;

  if (NULL == Buffer) {
    return 0;
  }

  Result = NumberOfBytes;

  while (NumberOfBytes--) {
    //
    // Wait for the serial port to be ready.
    //
    do {
      Data = IoRead8 ((UINT16) PcdGet64 (PcdSerialRegisterBase) + LSR_OFFSET);
    } while ((Data & LSR_RXDA) == 0);

    *Buffer++ = IoRead8 ((UINT16) PcdGet64 (PcdSerialRegisterBase));
  }

  return Result;
}

#if SERIAL_DEBUGGER_SUPPORT == 0
/*
  Common function to Read data from UART serial device, USB serial device and save the datas in buffer.

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8     *Buffer,
  IN  UINTN     NumberOfBytes
)
{
  if (FeaturePcdGet (PcdStatusCodeUseIsaSerial)) {
    UARTDbgIn (Buffer, NumberOfBytes);
  }

  //-if (FeaturePcdGet (PcdStatusCodeUseUsbSerial)) {
  //-  UsbDebugPortRead (Buffer, NumberOfBytes);
  //-}
  return RETURN_SUCCESS;
}
#endif

RETURN_STATUS
EFIAPI
PlatformHookSerialPortInitialize (
  VOID
  )
/*++

Routine Description:

  Initialization for SIO.

Arguments:

  FfsHeader   - FV this PEIM was loaded from.
  PeiServices - General purpose services available to every PEIM.

Returns:

  None

--*/
{
#if SERIAL_DEBUGGER_SUPPORT == 0
	SerialPortInitialize();
	
	return RETURN_SUCCESS;
#else
	  UINT32 Data32; //EIP134867
	  
	  // Force enabled UART decode start.
	  // Program and enable PMC Base.
	  IoWrite32 (PCI_IDX,  PCI_LPC_REG(R_PCH_LPC_PMC_BASE));
	  IoWrite32 (PCI_DAT,  (PMC_BASE_ADDRESS | B_PCH_LPC_PMC_BASE_EN));

	  // Enable COM1 for debug message output.
	  //EIP134867 >>
	  Data32 = MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
	  // Do not clear SUS Well Power Failure, General Reset Status and RTC Power Status bits
	  //EIP139043 >>
	  // BIOS should clear RTC_PWR_STS bit (PBASE + 0x20[2]) by writing a '0b' to this bit position
	//  Data32 &= (UINT32) ~(B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS | B_PCH_PMC_GEN_PMCON_RTC_PWR_STS);
	  Data32 &= (UINT32) ~(B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS);
	  //EIP139043 <<
	  Data32 |= BIT24;
	  MmioWrite32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, Data32);
	  //EIP134867 <<

	  if (PchStepping()>= PchB0)
	    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) V_PCH_ILB_IRQE_UARTIRQEN_IRQ4);
	  else
	    MmioOr8 (ILB_BASE_ADDRESS + R_PCH_ILB_IRQE, (UINT8) V_PCH_ILB_IRQE_UARTIRQEN_IRQ3);
	  MmioAnd32(IO_BASE_ADDRESS + 0x0520, (UINT32)~(0x00000187));
	  MmioOr32 (IO_BASE_ADDRESS + 0x0520, (UINT32)0x81); // UART3_RXD-L
	  MmioAnd32(IO_BASE_ADDRESS + 0x0530, (UINT32)~(0x00000007));
	  MmioOr32 (IO_BASE_ADDRESS + 0x0530, (UINT32)0x1); // UART3_RXD-L
	  MmioOr8 (PciD31F0RegBase + R_PCH_LPC_UART_CTRL, (UINT8) B_PCH_LPC_UART_CTRL_COM1_EN);
	  // Force enabled UART decode end.
	
	return RETURN_SUCCESS;
#endif
}
