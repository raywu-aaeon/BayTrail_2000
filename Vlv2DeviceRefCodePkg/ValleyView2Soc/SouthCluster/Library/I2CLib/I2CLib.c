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
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#else
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#endif
#include <PchRegs/PchRegsPcu.h> 
#include <PchRegs.h>
#include <PlatformBaseAddresses.h>
#include <PchRegs/PchRegsLpss.h> 
#ifdef ECP_FLAG
#include "I2CLib.h"
#else
#include <Library/I2CLib.h>
#endif
#include <Protocol/GlobalNvsArea.h>
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#endif

#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)

//#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#define PciD31F0RegBase             PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)


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
//
// LPIO I2C Module
// Memory Space Registers
//
#define R_PCH_LPIO_I2C_MEM_RESETS                 0x804 // Software Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_FUNC            BIT1  // Function Clock Domain Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_APB             BIT0  // APB Domain Reset
#define R_PCH_LPSS_I2C_MEM_PCP                    0x800 // Private Clock Parameters

STATIC UINTN mI2CBaseAddress = 0;
STATIC UINT16 mI2CSlaveAddress = 0;

UINT16      I2cMode=B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE ;

UINTN i2cNvsBaseAddress[] = {
  GLOBAL_NVS_OFFSET(LDMA2Addr),
  GLOBAL_NVS_OFFSET(I2C1Addr),
  GLOBAL_NVS_OFFSET(I2C2Addr),
  GLOBAL_NVS_OFFSET(I2C3Addr),
  GLOBAL_NVS_OFFSET(I2C4Addr),
  GLOBAL_NVS_OFFSET(I2C5Addr),
  GLOBAL_NVS_OFFSET(I2C6Addr),
  GLOBAL_NVS_OFFSET(I2C7Addr)
};


//get I2Cx controller base address (bar0)

UINTN
GetI2cBarAddr(
  IN    UINT8 BusNo
  )
{
  EFI_STATUS           Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  UINTN  AcpiBaseAddr;
  UINTN  PciMmBase=0;

  ASSERT(gBS!=NULL);

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  //
  //PCI mode from PEI ( Global NVS is not ready)
  //
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "GetI2cBarAddr() gEfiGlobalNvsAreaProtocolGuid:%r\n", Status));
    return 0;//not ready
  }

  AcpiBaseAddr =  *(UINTN*)((CHAR8*)GlobalNvsArea->Area + i2cNvsBaseAddress[BusNo+1]);
  //
  //PCI mode from DXE (global NVS protocal) to LPSS OnReadytoBoot(swith to ACPI)
  //
  if(AcpiBaseAddr==0) {
    PciMmBase = MmPciAddress (
                  mLpssPciDeviceList[BusNo+1].Segment,
                  mLpssPciDeviceList[BusNo+1].BusNum,
                  mLpssPciDeviceList[BusNo+1].DeviceNum,
                  mLpssPciDeviceList[BusNo+1].FunctionNum,
                  0
                  );
    DEBUG((EFI_D_ERROR, "\nGetI2cBarAddr() I2C Device %x %x %x PciMmBase:%x\n", \
           mLpssPciDeviceList[BusNo+1].BusNum, \
           mLpssPciDeviceList[BusNo+1].DeviceNum, \
           mLpssPciDeviceList[BusNo+1].FunctionNum, PciMmBase));

    if (MmioRead32 (PciMmBase) != 0xFFFFFFFF)    {
      if((MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)) {
        mLpssPciDeviceList[BusNo+1].Bar0 = MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR);     //get the address allocted.
        mLpssPciDeviceList[BusNo+1].Bar1 = MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1);
        DEBUG((EFI_D_ERROR, "GetI2cBarAddr() bar0:0x%x bar1:0x%x\n",mLpssPciDeviceList[BusNo+1].Bar0, mLpssPciDeviceList[BusNo+1].Bar1));
      }
    }
    AcpiBaseAddr =mLpssPciDeviceList[BusNo+1].Bar0;
  }
  //
  //ACPI mode from BDS: LPSS OnReadytoBoot
  //
  else {
    DEBUG ((EFI_D_INFO, "GetI2cBarAddr() NVS Varialable is updated by this LIB or LPSS  \n"));
  }
  DEBUG ((EFI_D_INFO, "GetI2cBarAddr() BusNo+1 0x%x AcpiBaseAddr:0x%x \n", BusNo+1, AcpiBaseAddr));
  return AcpiBaseAddr;
}



EFI_STATUS
ProgramPciLpssI2C (
  IN  UINT8 BusNo
  )
{
  UINT32 PmcBase;
  UINTN  PciMmBase=0;
  EFI_STATUS           Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

  UINT32 PMC_DIS[]= {
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6,
    B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7
  };

  DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() Start\n"));

  //
  // Set the VLV Function Disable Register to ZERO
  //
  PmcBase = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  if(MmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)&PMC_DIS[BusNo]) {
    DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() End:I2C[%x] is disabled\n",BusNo));
    return EFI_NOT_READY;
  }
  DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C()------------BusNo=%x,PMC=%x\n",BusNo,MmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)));

  //for(Index = 0; Index < LPSS_PCI_DEVICE_NUMBER; Index ++)
  {

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
    if (MmioRead32 (PciMmBase) != 0xFFFFFFFF)     {
      if((MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)) {
        mLpssPciDeviceList[BusNo+1].Bar0 = MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR);     //get the address allocted.
        mLpssPciDeviceList[BusNo+1].Bar1 = MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1);
        DEBUG((EFI_D_ERROR, "ProgramPciLpssI2C() bar0:0x%x bar1:0x%x\n",mLpssPciDeviceList[BusNo+1].Bar0, mLpssPciDeviceList[BusNo+1].Bar1));
      } else {
        ///
        /// Program BAR 0
        ///
        ASSERT (((mLpssPciDeviceList[BusNo+1].Bar0 & B_PCH_LPSS_I2C_BAR_BA) == mLpssPciDeviceList[BusNo+1].Bar0) && (mLpssPciDeviceList[BusNo+1].Bar0 != 0));
        MmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (mLpssPciDeviceList[BusNo+1].Bar0 & B_PCH_LPSS_I2C_BAR_BA));
        ///
        /// Program BAR 1
        ///
        ASSERT (((mLpssPciDeviceList[BusNo+1].Bar1 & B_PCH_LPSS_I2C_BAR1_BA) == mLpssPciDeviceList[BusNo+1].Bar1) && (mLpssPciDeviceList[BusNo+1].Bar1 != 0));
        MmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR1), (UINT32) (mLpssPciDeviceList[BusNo+1].Bar1 & B_PCH_LPSS_I2C_BAR1_BA));
        ///
        /// Bus Master Enable & Memory Space Enable
        ///

        MmioOr32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        ASSERT (MmioRead32 (mLpssPciDeviceList[BusNo+1].Bar0) != 0xFFFFFFFF);
      }
      ///
      /// Release Resets
      ///
      MmioWrite32 (mLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPIO_I2C_MEM_RESETS,(B_PCH_LPIO_I2C_MEM_RESETS_FUNC | B_PCH_LPIO_I2C_MEM_RESETS_APB));
      //
      // Activate Clocks
      //
      MmioWrite32 (mLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPSS_I2C_MEM_PCP,0x80020003);//No use for A0

      DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() Programmed()\n"));
    }
    //
    //BDS: already switched to ACPI mode
    //
    else {
      ASSERT(gBS!=NULL);
      Status = gBS->LocateProtocol (
                      &gEfiGlobalNvsAreaProtocolGuid,
                      NULL,
                      (VOID **) &GlobalNvsArea
                      );
      if (EFI_ERROR(Status)) {
        DEBUG ((EFI_D_INFO, "GetI2cBarAddr() gEfiGlobalNvsAreaProtocolGuid:%r\n", Status));
        return 0;//not ready
      }
      mLpssPciDeviceList[BusNo+1].Bar0 = *(UINT32*)((CHAR8*)GlobalNvsArea->Area + i2cNvsBaseAddress[BusNo+1]);
      DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C(): is switched to ACPI 0x:%x \n",mLpssPciDeviceList[BusNo+1].Bar0));
    }
  }
  DEBUG ((EFI_D_INFO, "ProgramPciLpssI2C() End\n"));

  return EFI_SUCCESS;
}

RETURN_STATUS
I2cDisable (
  VOID
  )
{
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  MmioWrite32 ( mI2CBaseAddress + R_IC_ENABLE, 0 );
  while ( 0 != ( MmioRead32 ( mI2CBaseAddress + R_IC_ENABLE_STATUS) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}

RETURN_STATUS
I2cEnable (
  VOID
  )
{
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  MmioWrite32 ( mI2CBaseAddress + R_IC_ENABLE, 1 );
  while ( 0 == ( MmioRead32 ( mI2CBaseAddress + R_IC_ENABLE_STATUS) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}
RETURN_STATUS
I2cBusFrequencySet (
  IN UINTN BusClockHertz
  )
{
  DEBUG((EFI_D_INFO,"InputFreq BusClockHertz: %d\r\n",BusClockHertz));
  //
  //  Set the 100 KHz clock divider according to SV result and I2C spec
  //
  MmioWrite32 ( mI2CBaseAddress + R_IC_SS_SCL_HCNT, (UINT16)0x214 );
  MmioWrite32 ( mI2CBaseAddress + R_IC_SS_SCL_LCNT, (UINT16)0x272 );
  //
  //  Set the 400 KHz clock divider according to SV result and I2C spec
  //
  MmioWrite32 ( mI2CBaseAddress + R_IC_FS_SCL_HCNT, (UINT16)0x50 );
  MmioWrite32 ( mI2CBaseAddress + R_IC_FS_SCL_LCNT, (UINT16)0xAD );

  switch ( BusClockHertz ) {
    case 100 * 1000:
      MmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x40);//100K
      I2cMode = V_SPEED_STANDARD;
      break;
    case 400 * 1000:
      MmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x32);//400K
      I2cMode = V_SPEED_FAST;
      break;
    default:
      MmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x09);//3.4M
      I2cMode = V_SPEED_HIGH;
  }

  //
  //  Select the frequency counter
  //  Enable restart condition,
  //  Enable master FSM, disable slave FSM
  //
  I2cMode |= B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;

  return EFI_SUCCESS;
}

EFI_STATUS
I2CInit (
  IN  UINT8  BusNo,
  IN  UINT16 SlaveAddress
  )
/*++

Routine Description:

  Initializes the host controller to execute I2C commands.


Returns:

  EFI_SUCCESS             Opcode initialization on the I2C host controller completed.
  EFI_DEVICE_ERROR        Device error, operation failed.

--*/
{
  EFI_STATUS Status=RETURN_SUCCESS;
  UINT32    NumTries = 0;
  UINTN    GnvsI2cBarAddr=0;
  //
  //  Verify the parameters
  //
  if (( 1023 < SlaveAddress) || ( 6 < BusNo)) {
    Status =  RETURN_INVALID_PARAMETER;
    DEBUG((EFI_D_INFO,"I2CInit Exit with RETURN_INVALID_PARAMETER\r\n"));
    return Status;
  }
  MmioWrite32 ( mI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );
  mI2CSlaveAddress = SlaveAddress;

  //
  //1.PEI: program and init ( before pci enumeration).
  //2.DXE:update address and re-init ( after pci enumeration).
  //3.BDS:update ACPI address and re-init ( after acpi mode is enabled).
  //
  if(mI2CBaseAddress == mLpssPciDeviceList[BusNo+1].Bar0) {
    // I2CInit is already  called
    GnvsI2cBarAddr=GetI2cBarAddr(BusNo);
    if(
      (GnvsI2cBarAddr == 0)||//PEI: Gnvs not ready.
      (GnvsI2cBarAddr == mI2CBaseAddress)//DXE and BDS
    ) {
      DEBUG((EFI_D_INFO,"I2CInit Exit with mI2CBaseAddress:%x == [%x].Bar0\r\n",mI2CBaseAddress,BusNo+1));
      return RETURN_SUCCESS;
    }
  }
  Status=ProgramPciLpssI2C(BusNo);
  if(Status!=EFI_SUCCESS)
    return Status;


  mI2CBaseAddress = (UINT32) mLpssPciDeviceList[BusNo+1].Bar0;
  DEBUG ((EFI_D_ERROR, "mI2CBaseAddress = 0x%x \n",mI2CBaseAddress));

  NumTries = 10000; /* 1 seconds */
  while (( 1 == ( MmioRead32 ( mI2CBaseAddress + R_IC_STATUS) & STAT_MST_ACTIVITY ))) {
    MicroSecondDelay(10);
    NumTries --;
    if(0 == NumTries) {
      DEBUG((EFI_D_INFO, "Try timeout\r\n"));
      return RETURN_DEVICE_ERROR;
    }
  }

  Status = I2cDisable();
  DEBUG((EFI_D_INFO, "I2cDisable Status = %r\r\n", Status));
  I2cBusFrequencySet(400 * 1000);//Set I2cMode
  //MmioWrite32( mI2CBaseAddress + R_IC_CLK_GATE, 0x01);//No use for A0
  MmioWrite32( mI2CBaseAddress + R_IC_INTR_MASK, 0x0);
  if ( 0x7f < SlaveAddress )
    SlaveAddress = ( SlaveAddress & 0x3ff ) | IC_TAR_10BITADDR_MASTER;
  MmioWrite32 ( mI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );
  MmioWrite32 ( mI2CBaseAddress + R_IC_RX_TL, 0);
  MmioWrite32 ( mI2CBaseAddress + R_IC_TX_TL, 0 );
  MmioWrite32 ( mI2CBaseAddress + R_IC_CON, I2cMode);
  Status = I2cEnable();

  DEBUG((EFI_D_INFO, "I2cEnable Status = %r\r\n", Status));
  MmioRead32 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
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
  UINT32 Count=0;

  Status = EFI_SUCCESS;

  //I2CInit(BusNo, SlaveAddress);
  //if(Status!=EFI_SUCCESS)
  //  return Status;

  ReceiveDataEnd = &ReadBuffer [ ReadBytes ];
  if( ReadBytes ) {

    ReceiveRequest = ReadBuffer;
    //DEBUG((EFI_D_INFO,"Read: ---------------%d bytes to RX\r\n",ReceiveDataEnd - ReceiveRequest));

    while ( (ReceiveDataEnd > ReceiveRequest) ||
            (ReceiveDataEnd > ReadBuffer)) {
      //  Check for NACK
      raw_intr_stat = (UINT16)MmioRead32 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead32 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_INFO,"TX ABRT ,%d bytes hasn't been transferred\r\n",ReceiveDataEnd - ReceiveRequest));
        break;
      }

      //  Determine if another byte was received
      I2cStatus = (UINT16)MmioRead32 ( mI2CBaseAddress + R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = (UINT16)MmioRead32 ( mI2CBaseAddress + R_IC_DATA_CMD );
        *ReadBuffer++ = (UINT8)ReceiveData;
        DEBUG((EFI_D_INFO,"MmioRead32 ,1 byte 0x:%x is received\r\n",ReceiveData));

      }

      if(ReceiveDataEnd==ReceiveRequest) {
        MicroSecondDelay ( FIFO_WRITE_DELAY );
        DEBUG((EFI_D_INFO,"ReceiveDataEnd==ReceiveRequest------------%x\r\n",I2cStatus & STAT_RFNE));
        Count++;
        if(Count<1024)//to avoid sys hung  without ul-pmc device  on RVP
          continue;//Waiting the last request to get data and make (ReceiveDataEnd > ReadBuffer) =TRUE.
        else
          break;
      }
      //  Wait until a read request will fit
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        DEBUG((EFI_D_INFO,"Wait until a read request will fit\r\n"));
        MicroSecondDelay ( 10 );
        continue;
      }
      //  Issue the next read request
      if(End && Start )
        MmioWrite32 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART|B_CMD_STOP);
      else if (!End && Start )
        MmioWrite32 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART);
      else if (End && !Start )
        MmioWrite32 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_STOP);
      else if (!End && !Start )
        MmioWrite32 ( mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD);
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd

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
  UINT32 Count=0;

  Status = EFI_SUCCESS;

  // Initialise I2C Device
  /*
  DEBUG((EFI_D_INFO,"BusNo/SlaveAddress/WriteBytes/I2C6_SDA/2C6_SCL:0x%x/0x%x/0x%x/0x%x/0x%x\r\n",\
      BusNo,SlaveAddress,WriteBytes,MmioRead32(0xfed0c000 + 0x0180),MmioRead32 (0xfed0c000 + 0x0160)));
  */

  Status=I2CInit(BusNo, SlaveAddress);
  if(Status!=EFI_SUCCESS)
    return Status;

  TransmitEnd = &WriteBuffer [ WriteBytes ];
  if( WriteBytes ) {

    //DEBUG((EFI_D_INFO,"Write: --------------%d bytes to TX\r\n",TransmitEnd - WriteBuffer));
    while ( TransmitEnd > WriteBuffer ) {
      I2cStatus = MmioRead32 ( mI2CBaseAddress + R_IC_STATUS );
      raw_intr_stat = (UINT16)MmioRead32 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead32 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {//if tx not full , will  send cmd  or continue to wait
        MicroSecondDelay ( FIFO_WRITE_DELAY );
        continue;
      }

      if(End && Start )
        MmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART|B_CMD_STOP);
      else if (!End && Start )
        MmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART);
      else if (End && !Start )
        MmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_STOP);
      else if (!End && !Start )
        MmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));
      // Add a small delay to work around some odd behavior being seen.  Without this delay bytes get dropped.
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd

      //time out
      while( 1 ) {
        raw_intr_stat = MmioRead16 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
        if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
          MmioRead16 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
          Status = RETURN_DEVICE_ERROR;
          DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
        }
        if( 0 == MmioRead16(mI2CBaseAddress + R_IC_TXFLR ) ) break;

        MicroSecondDelay ( FIFO_WRITE_DELAY );
        Count++;
        if(Count<1024)//to avoid sys hung without ul-pmc device on RVP
          continue;//Waiting the last request to get data and make (ReceiveDataEnd > ReadBuffer) =TRUE.
        else
          break;
      }//while( 1 )


    }

  }
  if(EFI_ERROR(Status))
    DEBUG((EFI_D_INFO,"ByteWriteI2C_Basic Exit with Status %r\r\n",Status));
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
  EFI_STATUS          Status;

  //DEBUG ((EFI_D_INFO, "ByteReadI2C:---offset:0x%x\n",Offset));
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
  EFI_STATUS          Status;

  //DEBUG ((EFI_D_INFO, "ByteWriteI2C:---offset/bytes/buf:0x%x,0x%x,0x%x,0x%x\n",Offset,WriteBytes,WriteBuffer,*WriteBuffer));
  Status = ByteWriteI2C_Basic(BusNo, SlaveAddress,1,&Offset,TRUE,FALSE);
  Status = ByteWriteI2C_Basic(BusNo, SlaveAddress,WriteBytes,WriteBuffer,FALSE,TRUE);

  return Status;
}
