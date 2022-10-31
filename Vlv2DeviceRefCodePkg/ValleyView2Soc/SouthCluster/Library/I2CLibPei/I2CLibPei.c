/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  I2CLib.c



--*/

#include "I2CDelayPei.h"
#include "I2CIoLibPei.h"
#include "I2CAccess.h"
#include "I2CLibPei.h"
#include <PlatformBaseAddresses.h>
#ifdef ECP_FLAG
#include "PchRegs.h"
#else
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/HobLib.h>
#endif
#include <PchRegs/PchRegsPcu.h> 
#include <PchRegs/PchRegsLpss.h> 


EFI_GUID  mI2CPeiInitGuid = {
  0x96DED71A, 0xB9E7, 0x4EAD, 0x96, 0x2C, 0x01, 0x69, 0x3C, 0xED, 0x2A, 0x64
};

typedef struct _LPSS_PCI_DEVICE_INFO {
  UINTN        Segment;
  UINTN        BusNum;
  UINTN        DeviceNum;
  UINTN        FunctionNum;
  UINTN        Bar0;
  UINTN        Bar1;
} LPSS_PCI_DEVICE_INFO;

LPSS_PCI_DEVICE_INFO  mLpssPciDeviceList[] = {
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1, PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC, 0xFE900000, 0xFE908000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0, 0xFE910000, 0xFE918000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1, 0xFE920000, 0xFE928000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2, 0xFE930000, 0xFE938000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3, 0xFE940000, 0xFE948000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4, 0xFE950000, 0xFE958000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5, 0xFE960000, 0xFE968000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6, 0xFE970000, 0xFE978000}
};
#define LPSS_PCI_DEVICE_NUMBER  sizeof(mLpssPciDeviceList)/sizeof(LPSS_PCI_DEVICE_INFO)


#define R_PCH_LPIO_I2C_MEM_RESETS                 0x804 // Software Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_FUNC            BIT1  // Function Clock Domain Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_APB             BIT0  // APB Domain Reset
#define R_PCH_LPSS_I2C_MEM_PCP                    0x800 // Private Clock Parameters
UINT16 I2CGPIO[]= {
  /*
   19.1.6  I2C0
   I2C0_SDA-OD-O -    write 0x2003CC81 to IOBASE + 0x0210
   I2C0_SCL-OD-O -    write 0x2003CC81 to IOBASE + 0x0200
   */
  0x0210,
  0x0200,

  /*
  19.1.7  I2C1
  I2C1_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01F0
  I2C1_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01E0
  */
  0x01F0,
  0x01E0,

  /*
  19.1.8  I2C2
  I2C2_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01D0
  I2C2_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01B0
  */
  0x01D0,
  0x01B0,

  /*
  19.1.9  I2C3
  I2C3_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0190
  I2C3_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01C0
  */
  0x0190,
  0x01C0,

  /*
  19.1.10 I2C4
  I2C4_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01A0
  I2C4_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0170
  */
  0x01A0,
  0x0170,

  /*
  19.1.11 I2C5
  I2C5_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0150
  I2C5_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0140
  */
  0x0150,
  0x0140,

  /*
  19.1.12 I2C6
  I2C6_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0180
  I2C6_SCL-OD-O/I -  write 0x2003CC81 to IOBASE + 0x0160
  */
  0x0180,
  0x0160
};

EFI_STATUS
EFIAPI
IntelI2CPeiLibConstructor (
  VOID
  )
{
  UINTN Index;
  for (Index = 0; Index <   sizeof(I2CGPIO)/sizeof(UINT16); Index ++) {
    I2CLibPeiMmioWrite32(IO_BASE_ADDRESS+I2CGPIO[Index], 0x2003CC81);
  }


  return EFI_SUCCESS;
}


EFI_STATUS
ProgramPciLpssI2C (
  IN  UINT8 BusNo
  )
{
  UINT32       PmcBase;
  UINT32       DevID;
  UINTN        PciMmBase=0;

  UINT32 PMC_DIS[]= {
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7
  };

  DEBUG ((EFI_D_INFO, "Pei ProgramPciLpssI2C() Start\n"));
  //
  // Set the VLV Function Disable Register to ZERO
  //
  PmcBase         = I2CLibPeiMmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  if(I2CLibPeiMmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)&PMC_DIS[BusNo]) {
    DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() enable:I2C[%x]\n",BusNo));
    I2CLibPeiMmioWrite32(
      PmcBase+R_PCH_PMC_FUNC_DIS,
      I2CLibPeiMmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)&(~PMC_DIS[BusNo])
      );
  }
    

  // for(Index = 0; Index < LPSS_PCI_DEVICE_NUMBER; Index ++) {

  PciMmBase = MmPciAddress (
               mLpssPciDeviceList[BusNo+1].Segment,
               mLpssPciDeviceList[BusNo+1].BusNum,
               mLpssPciDeviceList[BusNo+1].DeviceNum,
               mLpssPciDeviceList[BusNo+1].FunctionNum,
               0
               );
   DEBUG((EFI_D_ERROR, "Program Pci Lpss I2C Device  %x %x %x PciMmBase:%x\n", \
        mLpssPciDeviceList[BusNo+1].BusNum, \
        mLpssPciDeviceList[BusNo+1].DeviceNum, \
        mLpssPciDeviceList[BusNo+1].FunctionNum, PciMmBase));

    ///
    /// Check if device present
    ///
    
   DevID =  I2CLibPeiMmioRead32(PciMmBase);
    
   if (DevID  != 0xFFFFFFFF)  {
      if(!(I2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)) {
        ///
        /// Program BAR 0
        ///
        I2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (mLpssPciDeviceList[BusNo+1].Bar0 & B_PCH_LPSS_I2C_BAR_BA));
        //DEBUG ((EFI_D_ERROR, "mI2CBaseAddress1 = 0x%x \n",I2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR)));
        ///
        /// Program BAR 1
        ///
        I2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR1), (UINT32) (mLpssPciDeviceList[BusNo+1].Bar1 & B_PCH_LPSS_I2C_BAR1_BA));
        //DEBUG ((EFI_D_ERROR, "mI2CBaseAddress1 = 0x%x \n",I2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1)));
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        I2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        //ASSERT (I2CLibPeiMmioRead32 (Bar0) != 0xFFFFFFFF);
      }
      ///
      /// Release Resets
      ///
      I2CLibPeiMmioWrite32 (mLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPIO_I2C_MEM_RESETS,(B_PCH_LPIO_I2C_MEM_RESETS_FUNC | B_PCH_LPIO_I2C_MEM_RESETS_APB));
      //
      // Activate Clocks
      //
      I2CLibPeiMmioWrite32 (mLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPSS_I2C_MEM_PCP,0x80020003);//No use for A0

      //DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() Programmed()\n"));
   }

// }
   DEBUG ((EFI_D_INFO, "Pei ProgramPciLpssI2C() End\n"));

   return EFI_SUCCESS;
}

EFI_STATUS
I2cDisable (
  IN UINT8 BusNo
  )
{
  UINTN   mI2CBaseAddress;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  mI2CBaseAddress = mLpssPciDeviceList[BusNo+1].Bar0;
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_ENABLE, 0 );
  while ( 0 != ( I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return EFI_NOT_READY;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
I2cEnable (
  IN UINT8 BusNo
  )
{
  UINTN   mI2CBaseAddress;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  mI2CBaseAddress = mLpssPciDeviceList[BusNo+1].Bar0;
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_ENABLE, 1 );
  while ( 0 == ( I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return EFI_NOT_READY;
  }
  return EFI_SUCCESS;
}


/**
  Set the I2C controller bus clock frequency.

  @param[in] This           Address of the library's I2C context structure
  @param[in] PlatformData   Address of the platform configuration data
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval RETURN_SUCCESS      The bus frequency was set successfully.
  @retval RETURN_UNSUPPORTED  The controller does not support this frequency.

**/
EFI_STATUS
I2cBusFrequencySet (
  IN UINTN   mI2CBaseAddress,
  IN UINTN   BusClockHertz,
  IN UINT16  *I2cMode
  )
{
  DEBUG((EFI_D_INFO,"InputFreq BusClockHertz: %d\r\n",BusClockHertz));

  *I2cMode = B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;

  //
  //  Set the 100 KHz clock divider
  //
  //  From Table 10 of the I2C specification
  //
  //    High: 4.00 uS
  //    Low:  4.70 uS
  //
  //DEBUG((EFI_D_INFO, "100khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_SS_SCL_HCNT, (UINT16)0x214 );
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_SS_SCL_LCNT, (UINT16)0x272 );
  //
  //    Set the 400 KHz clock divider
  //
  //    From Table 10 of the I2C specification
  //
  //      High: 0.60 uS
  //      Low:  1.30 uS
  //
  //DEBUG((EFI_D_INFO, "400khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_FS_SCL_HCNT, (UINT16)0x50 );
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_FS_SCL_LCNT, (UINT16)0xAD );

  switch ( BusClockHertz ) {
    case 100 * 1000:
      I2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x40);//100K
      *I2cMode |= V_SPEED_STANDARD;
      break;
    case 400 * 1000:
      I2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x32);//400K
      *I2cMode |= V_SPEED_FAST;
      break;
    default:
      I2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x09);//3.4M
      *I2cMode |= V_SPEED_HIGH;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
I2CInit (UINT8 BusNo, UINT16 SlaveAddress)
{
  EFI_STATUS Status;
  UINT32        NumTries = 0;
  UINTN          mI2CBaseAddress;
  UINT16        I2cMode;
  UINTN         PciMmBase=0;


  PciMmBase = MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_PCH,
                PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                (BusNo + 1),
                0
                );

  mI2CBaseAddress = I2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR) & B_PCH_LPSS_I2C_BAR_BA;

  //
  //  Verify the parameters
  //
  if ( 1023 < SlaveAddress ) {
    Status =  EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_INFO,"I2cStartRequest Exit with Status %r\r\n", Status));
    return Status;
  }
  I2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );
  //mI2CSlaveAddress = SlaveAddress;

  if(mI2CBaseAddress == mLpssPciDeviceList[BusNo+1].Bar0) {
    return EFI_SUCCESS;
  }
  ProgramPciLpssI2C(BusNo);
  
  mI2CBaseAddress = mLpssPciDeviceList[BusNo+1].Bar0;
  DEBUG ((EFI_D_ERROR, "mI2CBaseAddress = 0x%x \n",mI2CBaseAddress));
  NumTries = 10000; /* 1 seconds */
  while (( 1 == ( I2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_STATUS) & STAT_MST_ACTIVITY ))) {
    MicroSecondDelay(10);
    NumTries --;
    if(0 == NumTries)
      return EFI_DEVICE_ERROR;
  }
  

  Status = I2cDisable ( BusNo);
  DEBUG((EFI_D_INFO, "I2cDisable Status = %r\r\n", Status));

  I2cBusFrequencySet(mI2CBaseAddress, 400 * 1000, &I2cMode);//Set I2cMode

  //MmioWrite32( mI2CBaseAddress + R_IC_CLK_GATE, 0x01);//No use for A0
  I2CLibPeiMmioWrite16( mI2CBaseAddress + R_IC_INTR_MASK, 0x0);
  if ( 0x7F < SlaveAddress ) {
    SlaveAddress = ( SlaveAddress & 0x3ff ) | IC_TAR_10BITADDR_MASTER;
  }
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_TAR, (UINT16) SlaveAddress );
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_RX_TL, 0);
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_TX_TL, 0 );
  I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_CON, I2cMode);

  Status = I2cEnable(BusNo);
//  DEBUG((EFI_D_INFO, "I2cEnable Status = %r\r\n", Status));
  I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
  return EFI_SUCCESS;
}
EFI_STATUS ByteReadI2C_Basic(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress,
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer,
  IN  UINT8 Start,
  IN  UINT8 End
  )
/*++

Routine Description:

  Reads a Byte from I2C Device


Input:

  BusNo             I2C Bus no to which the I2C device has been connected
  SlaveAddress      Device Address from which the byte value has to be read
  Offset            Offset from which the data has to be read
  *Byte             Address to which the value read has to be stored

Return:

  EFI_SUCCESS       IF the byte value has been successfully read
  EFI_DEVICE_ERROR  Operation Failed, Device Error

--*/
{

  EFI_STATUS Status;
  UINT32 I2cStatus;
  UINT16 ReceiveData;
  UINT8 *ReceiveDataEnd;
  UINT8 *ReceiveRequest;
  UINT16 raw_intr_stat;
  UINTN   mI2CBaseAddress;

  mI2CBaseAddress = (UINT32)  (0xFE910000+ BusNo *0x10000);

  Status = EFI_SUCCESS;

  I2CInit(BusNo, SlaveAddress);

  ReceiveDataEnd = &ReadBuffer [ ReadBytes ];
  if( ReadBytes ) {
    ReceiveRequest = ReadBuffer;
//    DEBUG((EFI_D_INFO,"Read: ---------------%d bytes to RX\r\n",ReceiveDataEnd - ReceiveRequest));

    while ( (ReceiveDataEnd > ReceiveRequest) ||
            (ReceiveDataEnd > ReadBuffer)) {
      // Check for NACK
      raw_intr_stat = I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
//        DEBUG((EFI_D_INFO,"TX ABRT ,%d bytes hasn't been transferred\r\n",ReceiveDataEnd - ReceiveRequest));
        break;
      }

      //    Determine if another byte was received
      I2cStatus = I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_DATA_CMD );
        *ReadBuffer++ = (UINT8)ReceiveData;
//        DEBUG((EFI_D_INFO,"MmioRead32 ,1 byte 0x:%x is received\r\n",ReceiveData));
      }

      if(ReceiveDataEnd==ReceiveRequest) {
        //static UINT32 Count=0;
        //Count++;
        //if(Count<128)
        continue;//Waiting the last request to get data and make (ReceiveDataEnd > ReadBuffer) =TRUE.
        //else
        //    break;
      }
      //Wait until a read request will fit
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        MicroSecondDelay ( 10 );
        continue;
      }
      // Issue the next read request
      if(End && Start ) {
        I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART|B_CMD_STOP);
      } else if (!End && Start ) {
        I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART);
      } else if (End && !Start ) {
        I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_STOP);
      } else if (!End && !Start ) {
        I2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD);
      }
      ReceiveRequest += 1;
    }
    //local_irq_restore(eflags);
  }
  return Status;

}

EFI_STATUS ByteWriteI2C_Basic(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress,
  IN  UINTN WriteBytes,
  IN  UINT8 *WriteBuffer,
  IN  UINT8 Start,
  IN  UINT8 End
  )
/*++

Routine Description:

  Writes a Byte to I2C Device


Input:

  BusNo             I2C Bus no to which the I2C device has been connected
  SlaveAddress      Device Address from which the byte value has to be read
  Offset            Offset from which the data has to be read
  *Byte             Address to which the byte value has to be written

Return:

  EFI_SUCCESS       IF the byte value written successfully
  EFI_DEVICE_ERROR  Operation Failed, Device Error

--*/
{

  EFI_STATUS Status;
  UINT32 I2cStatus;
  UINT8 *TransmitEnd;
  UINT16 raw_intr_stat;

  UINTN   mI2CBaseAddress;

  mI2CBaseAddress = (UINT32)  0xFE910000+ BusNo *0x10000;

  Status = EFI_SUCCESS;

  // Initialise I2C Device
/*
  DEBUG((EFI_D_ERROR,"BusNo/SlaveAddress/WriteBytes/I2C6_SDA/2C6_SCL:0x%x/0x%x/0x%x/0x%x/0x%x\r\n",\
      BusNo,SlaveAddress,WriteBytes,I2CLibPeiMmioRead32(0xfed0c000 + 0x0180),I2CLibPeiMmioRead32 (0xfed0c000 + 0x0160)));
*/

  I2CInit(BusNo, SlaveAddress);

  TransmitEnd = &WriteBuffer [ WriteBytes ];
  if( WriteBytes ) {

//    DEBUG((EFI_D_INFO,"Write: --------------%d bytes to TX\r\n",TransmitEnd - WriteBuffer));

    while ( TransmitEnd > WriteBuffer ) {
      I2cStatus = I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_STATUS );
      raw_intr_stat = I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        I2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        continue;
      }
      if(End && Start )
        I2CLibPeiMmioWrite16 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART|B_CMD_STOP);
      else if (!End && Start )
        I2CLibPeiMmioWrite16 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART);
      else if (End && !Start )
        I2CLibPeiMmioWrite16 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_STOP);
      else if (!End && !Start )
        I2CLibPeiMmioWrite16 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));

      // Add a small delay to work around some odd behavior being seen.  Without this delay bytes get dropped.
      MicroSecondDelay ( FIFO_WRITE_DELAY );
    }

  }
  if(EFI_ERROR(Status))
    DEBUG((EFI_D_INFO,"I2cStartRequest Exit with Status %r\r\n",Status));
  return Status;
}

EFI_STATUS ByteReadI2C(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress,
  IN  UINT8 Offset,
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer
  )
{
  EFI_STATUS        Status;

//    DEBUG ((EFI_D_ERROR, "ByteReadI2C:---offset:0x%x\n",Offset));
  Status = ByteWriteI2C_Basic(BusNo, SlaveAddress,1,&Offset,TRUE,FALSE);
  Status = ByteReadI2C_Basic(BusNo, SlaveAddress,ReadBytes,ReadBuffer,TRUE,TRUE);

  return Status;
}

EFI_STATUS ByteWriteI2C(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress,
  IN  UINT8 Offset,
  IN  UINTN WriteBytes,
  IN  UINT8 *WriteBuffer
  )
{
  EFI_STATUS        Status;

//    DEBUG ((EFI_D_ERROR, "ByteWriteI2C:---offset/bytes/buf:0x%x,0x%x,0x%x,0x%x\n",Offset,WriteBytes,WriteBuffer,*WriteBuffer));
  Status = ByteWriteI2C_Basic(BusNo, SlaveAddress,1,&Offset,TRUE,FALSE);
  Status = ByteWriteI2C_Basic(BusNo, SlaveAddress,WriteBytes,WriteBuffer,FALSE,TRUE);

  return Status;
}
