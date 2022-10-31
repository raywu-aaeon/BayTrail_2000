/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved    *;
;*    This software and associated documentation (if any) is furnished    *;
;*    under a license and may only be used or copied in accordance        *;
;*    with the terms of the license. Except as permitted by such          *;
;*    license, no part of this software or documentation may be           *;
;*    reproduced, stored in a retrieval system, or transmitted in any     *;
;*    form or by any means without the express written consent of         *;
;*    Intel Corporation.                                                  *;
;*                                                                        *;
;*                                                                        *;
;**************************************************************************/
/*++
   Battery charging and AC adaptor
--*/

scope(\_SB.I2C1)
{
  // Battery Device
  Name(SMFG,ResourceTemplate ()
  {
    I2CSerialBus(0x55,        //SlaveAddress
                 ,                       //SlaveMode: default to ControllerInitiated
                 100000,                 //ConnectionSpeed: in Hz
                 ,                       //Addressing Mode: default to 7 bit
                 "\\_SB.I2C1",           //ResourceSource: I2C bus controller name
                 ,                       //ResourceSourceIndex: defaults to 0
                 ,                       //ResourceUsage: Defaults to ResourceConsumer
                 ,                       //Descriptor Name: creates name for offset of resource descriptor
                )  //VendorData
  })

  OperationRegion(DVFG,GenericSerialBus , 0x00, 0x100) // GenericSerialBus device at command
  Field(DVFG, BufferAcc, NoLock, Preserve)
  {
    Connection(SMFG),                                // Use the Resource Descriptor defined above
    Offset(0x0),
    AccessAs(BufferAcc, AttribBytes(48)),            // Use the GenericSerialBus Read/Write Block protocol
    FGC0, 8,
    FGC1, 8
  }

  Field(DVFG, BufferAcc, NoLock, Preserve)
  {
    Connection(SMFG),                                // Use the Resource Descriptor defined above
    Offset(0x3c),
    AccessAs(BufferAcc, AttribBytes(20)),            // Use the GenericSerialBus Read/Write Block protocol
    FGEX, 8,
  }

  Name(SMCG,ResourceTemplate ()
  {
    I2CSerialBus(0x6B,      //SlaveAddress
                 ,                       //SlaveMode: default to ControllerInitiated
                 100000,                 //ConnectionSpeed: in Hz
                 ,                       //Addressing Mode: default to 7 bit
                 "\\_SB.I2C1",           //ResourceSource: I2C bus controller name
                 ,                       //ResourceSourceIndex: defaults to 0
                 ,                       //ResourceUsage: Defaults to ResourceConsumer
                 ,                       //Descriptor Name: creates name for offset of resource descriptor
                )  //VendorData
  })
  OperationRegion(DVCG,GenericSerialBus , 0x00, 0x100) // GenericSerialBus device at command
  Field(DVCG, BufferAcc, Lock, Preserve)
  {
    Connection(SMCG), // Use the Resource Descriptor defined above
    Offset(0x0),
    AccessAs(BufferAcc, AttribBytes(11)), // Use the GenericSerialBus Read/Write Bytes protocol
    CGC0, 8, // Virtual register at command value 0.
    CGC1, 8
  }

             Field(DVCG, BufferAcc, NoLock, Preserve)
  {
    Connection(SMCG),
    AccessAs(BufferAcc, AttribByte),
    FLD0, 8, // Virtual register at command value 0.
    FLD1, 8, // Virtual register at command value 1.
    FLD2, 8, // Virtual register at command value 2.
    FLD3, 8, // Virtual register at command value 3.
    FLD4, 8, // Virtual register at command value 4.
    FLD5, 8, // Virtual register at command value 5.
    FLD6, 8, // Virtual register at command value 6.
    FLD7, 8, // Virtual register at command value 7.
    FLD8, 8, // Virtual register at command value 8.
    FLD9, 8, // Virtual register at command value 9.
    FLDA, 8  // Virtual register at command value A.
  }

  Device (BAT2)
  {
    Name(_HID,EISAID("PNP0C0A"))
    Name(_UID,1)
    Name(BCCC, 0x80)   //charging current .  default 0x80=2.500A
    Name(BCCE, 1)      //charging enable flag.  0-disable charging 1-enable charging
    Name(BCCV, 0x37)      //charging voltage.  0x37=4.2V
    Name(BCCL, 0xFFFFFFFF)     //battery cycle (can not use 0 as default since it will cause not update in cycle 0)
    Name(BCLP, 5866)   //Last fully charged Capacity TODO: update capacity

    Name(BTPC,0x00)   //Battery Trip point.   0--clear trip point

    // Create the GenericSerialBus data buffer for Fuel Gauge chip
    Name(BFFG, Buffer(50) {0xff}) // 48+2
    CreateByteField(BFFG, 0x00, STAT) // STAT = Status (Byte)
    CreateBytefield(BFFG, 0x01, LEN)  // LEN = Length (Byte)
    CreateWordField(BFFG, 0x02, CNTL) // 16, Control( )
    CreateWordField(BFFG, 0x04, ATRA) // 16, AtRate( )
    CreateWordField(BFFG, 0x06, ARTE) // 16, AtRateTimeToEmpty( ) Minutes
    CreateWordField(BFFG, 0x08, TEMP) // 16, Temperature( ) 0.1 K
    CreateWordField(BFFG, 0x0A, VOLT) // 16, Voltage( ) mV
    CreateWordField(BFFG, 0x0C, FLAG) // 16, Flags( )
    CreateWordField(BFFG, 0x0E, NACT) // 16, NominalAvailableCapacity( ) mAh
    CreateWordField(BFFG, 0x10, FACT) // 16, FullAvailableCapacity( )
    CreateWordField(BFFG, 0x12, RECT) // 16, RemainingCapacity( )
    CreateWordField(BFFG, 0x14, FCCT) // 16, FullChargeCapacity( )
    CreateWordField(BFFG, 0x16, AVRC) // 16, AverageCurrent( ) mA
    CreateWordField(BFFG, 0x18, TTET) // 16, TimeToEmpty( )
    CreateWordField(BFFG, 0x1A, TTFL) // 16, TimeToFull( )
    CreateWordField(BFFG, 0x1C, STDC) // 16, StandbyCurrent( )
    CreateWordField(BFFG, 0x1E, STTE) // 16, StandbyTimeToEmpty( )
    CreateWordField(BFFG, 0x20, MALC) // 16, MaxLoadCurrent( )
    CreateWordField(BFFG, 0x22, MLTE) // 16, MaxLoadTimeToEmpty( )
    CreateWordField(BFFG, 0x24, AVEE) // 16, AvailableEnergy( )
    CreateWordField(BFFG, 0x26, AVEP) // 16, AveragePower( )
    CreateWordField(BFFG, 0x28, TECP) // 16, TimeToEmptyAtConstantPower( )
    CreateWordField(BFFG, 0x2A, RSVD) // 16, Reserved
    CreateWordField(BFFG, 0x2C, CYLC) // 16, CycleCount( ) Counts
    CreateWordField(BFFG, 0x2E, SOCH) // 16, StateOfCharge( ) %

    Name(BXFG, Buffer(22) {0xff}) // 20+2
    CreateByteField(BXFG, 0x00, STAX)  // STAX = Status (Byte)
    CreateBytefield(BXFG, 0x01, LENX)  // LENX = Length (Byte)
    CreateWordField(BXFG, 0x02, DSCP)  // 16, DesignCapacity()  mAh

    Name(BFCG, Buffer(13) {}) // 11+2
    CreateByteField(BFCG, 0x00, CGST) // STAT = Status (Byte)
    CreateBytefield(BFCG, 0x01, CLEN) // LEN = Length (Byte)
    CreateBytefield(BFCG, 0x02, ISCR) // 8 REG00
    CreateBytefield(BFCG, 0x03, POCR) // 8 REG01
    CreateBytefield(BFCG, 0x04, CCCR) // 8 REG02
    CreateBytefield(BFCG, 0x05, TCCR) // 8 REG03
    CreateBytefield(BFCG, 0x06, CVCR) // 8 REG04
    CreateBytefield(BFCG, 0x07, CTCR) // 8 REG05
    CreateBytefield(BFCG, 0x08, TRCR) // 8 REG06
    CreateBytefield(BFCG, 0x09, MOCR) // 8 REG07
    CreateBytefield(BFCG, 0x0A, SSRE) // 8 REG08 //Bit5/bit4 = 00-Not Charging, 01-Pre-charge, 10-Fast Charging, 11-Charge Done.
    CreateBytefield(BFCG, 0x0B, FALT) // 8 REG09
    CreateBytefield(BFCG, 0x0C, VREV) // 8 REG0A

    Name(BUFF, Buffer(3) {00,1,00})
    CreateByteField(BUFF, 0x00, BYAT) // offset 0, STAT = Status (Byte)
    // offset 1 is the data size by byte ==1 as always by byte read/write.
    CreateByteField(BUFF, 0x02, DATA) // offset Data = the byte value will write to chipset

    Name(BIXP, Package()                             //Data Package for _BIX
    {
      0x00000000,                                      // Revision, current reversion is 0 according to ACPI5.0
      0x00000001,                                      // Power Unit 0 = mWh, 1=mAh.
      0xFFFFFFFF,                                      // Unknown Design Capacity.
      0xFFFFFFFF,                                      // Unknown Last Full Charge.
      0x00000001,                                      // Secondary Battery Technology.
      0xFFFFFFFF,                                      // Unknown Design Voltage.
      0x0000000A,                                      // 10% Warning Level.
      0x00000004,                                      // 4% Low Level.
      0x00000000,                                      // Cycle Count
      95000,                                      // Measurement Accuracy //Integer (DWORD) The value 80000 would mean 80% accuracy
      0xFFFFFFFF,                                      // Max Sampling Time //Integer (DWORD)
      0xFFFFFFFF,                                      // Min Sampling Time //Integer (DWORD)
      35000,                                      // Max Averaging Interval //Integer (DWORD)   set to 35000ms
      25000,                                      // Min Averaging Interval //Integer (DWORD)   set to 25000ms
      0x00000001,                                      // Battery Capacity Granularity 1 //Integer (DWORD)
      0x00000001,                                      // Battery Capacity Granularity 2 //Integer (DWORD)
      "SR Real Battery",                               // Model Number //String (ASCIIZ)
      "123456789",                                     // Serial Number //String (ASCIIZ)
      "LION",                                          // Battery Type //String (ASCIIZ)
      "Intel SR 1"
    })                                   // OEM Information //String (ASCIIZ)

    Name(BSTP,Package()
    {
      0x00000000,  // Battery State.
      //Bit0 1 indicates the battery is discharging.
      //Bit1 1 indicates the battery is charging.
      //Bit2 1 indicates the battery is in the critical energy state
      0xFFFFFFFF,  // Battery Present Rate. (in mA)
      0xFFFFFFFF,  // Battery Remaining Capacity. (in mWh)
      0xFFFFFFFF  // Battery Present Voltage. (in mV)
    })

    Method(_BIX,0)
    {
      Store(\_SB.I2C1.FGEX, BXFG)                 //bytes read FG, 20 bytes
      Store(\_SB.I2C1.FGC0, BFFG)                 //bytes read FG, 48 bytes
      Store(\_SB.I2C1.CGC0, BFCG)                 //bytes read CG, 11 bytes

      If(LNotEqual(STAX, 0x01))
      {
        store(DSCP, Index(BIXP,2))              // TODO: need to update battery capacity for BYT

        //PC++       32WH    3.7V   8648mAh     1C=8648mA
        //RVP1.4     14.8WH  3.7V   4000mAh     1C=4000mA
        //Dali PC2   22WH    3.75V  5866mAH     1C=5866mA
        If (LOr(LGreater(DSCP, 8000), LLess(DSCP, 4000)))
        {
          store(3700, Index(BIXP,5))           // Design voltage 3700mV
        }
        Else
        {
          store(3750, Index(BIXP,5))           // Design voltage 3750mV
        }
      }

      If(LAnd(LNotEqual(STAT, 0x01), LNotEqual(CGST, 0x01)))
      {
        If (LNotEqual(CYLC,  BCCL ))
        {
          store(CYLC, BCCL)                   // update cycle number
          store(FCCT, BCLP)                   // update Last fully charged capacity when cycle number changes
        }
        store(BCLP, Index(BIXP,3))              // Last fully charged capacity
        store(CYLC, Index(BIXP,8))              // Cycle count
      }

      Divide(DSCP, 10, Local0, Local1)            // 10% * Design Capacity
      Store(Local1, Index(BIXP,6))                // Warning Level. 10%
      Divide(DSCP, 50, Local0, Local1)            // 2% * Design Capcity
      Store(Local1, Index(BIXP,7))                // Low Level  2%

      Return (BIXP)
    }

    Method(_BST,0)
    {
      Store(\_SB.I2C1.FGC0, BFFG)                   //bytes read FG, 48 bytes
      Store(\_SB.I2C1.CGC0, BFCG)                   //bytes read CG, 11 bytes

      If (LEqual(BCCE, 0))                          //force to disable charging by DPTF
      {
        Store(POCR, Local0)                       //reg01
        And(Local0, 0xCF, Local0)                 //clear bit[5:4]    0xCF=b'1100,1111
        Store(Local0, DATA)
        Store(BUFF,\_SB.I2C1.FLD1)                //bit[5:4]==b'00 charge disable
      } else
      {
        Store(0x11, DATA)
        Store(BUFF,\_SB.I2C1.FLD1)                //Power-on Config: Charge battery & min sys vol limit is 3.0V
      }

      Store(BCCV, DATA)                             //TODO: Need to remove this line if S5 charging driver have REG0x0 init.
      Store(BUFF,\_SB.I2C1.FLD0)                    //set the charge voltage

      Store(BCCC, DATA)                             //get current from DPTF
      Store(BUFF,\_SB.I2C1.FLD2)                    //Set charging current

      Store(0x11, DATA)                             //set charging termination current 256mA
      Store(BUFF,\_SB.I2C1.FLD3)                    //set Pre-charging current limit 256mA

      Store(0xB2, DATA)                             //Charging voltage to 4.208V
      Store(BUFF,\_SB.I2C1.FLD4)                    //

      Store(0x8A, DATA)
      Store(BUFF,\_SB.I2C1.FLD5)                    //disable watchdog timer


      If(LAnd(LNotEqual(STAT, 0x01), LNotEqual(CGST, 0x01)))
      {

        //BSTP.index0-> Bit0 =1 indicates the battery is discharging.
        //BSTP.index0-> Bit1 =1 indicates the battery is charging.
        //BSTP.index0-> Bit2 =1 indicates the battery is in the critical energy state
        //AVRC.bit31 = 1 discharging.
        //SSRE.bit4,bit5-> 00-Not Charging, 01-Pre-charge, 10-Fast Charging, 11-Charge Done.

        ShiftRight(SSRE,4,Local0)
        And(Local0, 0x0003, Local0)
        If(Lor(LEqual(Local0, 0x01), LEqual(Local0, 0x02)))     //charging
        {
          Store(0x0002,Local1)
        }

        if (LEqual(local0, 0x00))                   //'not charging' in CG means discharging
        {
          Store(0x0001,Local1)
        }

        If (LEqual(Local0,0x03))
        {
          Store(0x0000, Local1)                     //charging done by clearing bit2 and bit1
        }
        Store(Local1,Index(BSTP,0))

        Store(AVRC,Local1)
        if(And(Local1,0x8000))                      //check if discharging
        {
          Subtract (0xFFFF, Local1, Local1)
        }  //end if
        Store(Local1,Index(BSTP,1))                 //at rate (MA)

        Store(RECT,Index(BSTP,2))                   //remaining capacity

        Store(VOLT,Index(BSTP,3))
      }

      //If (CondRefOf(\_SB.TCHG, Local0)) { //Before notifying, check if TCHG device is present or not. if DPTF is disabled, then this device will not be present
      //    Notify(\_SB.TCHG, 0x80)   //debug TCHG
      //}
      Return(BSTP)
    } //_BST

    Method(_STA,0)
    {
      If(LEqual(BCSL , 1))                              // Battery charging solution is ULPMC
      {
        Return(0x0000)
      }

      Store(\_SB.I2C1.FGC0, BFFG)
      If(LNotEqual(STAT, 0x01))
      {
        Store(FLAG, Local0)
        ShiftRight(Local0, 3, Local0)
        And(Local0, 0x1, Local0)                        //get bit3 of flag(). It is BAT_DET
        If( LEqual(Local0, 1))
        {
          Return(0x001F)
        }
      }
      Return(0x0000)
    } //_STA

    Method(_PCL,0)
    {
      Return(\_SB)
    }
  }   //Device(BATC)
}   //  Device (I2C1)


scope(\_SB)
{
  Device(ADP2)
  {
    Name(_HID,"ACPI0003")    //only one ACPI0003 is allowed in system
    // Return the value that determines if running with AC or not.
    Method(_PSR,0)
    {
      Store(\_SB.I2C1.BAT2.SSRE, Local0)
      ShiftLeft(Local0 ,1, Local1)
      ShiftRight(Local1 ,7, Local1)  // bit6 respensts the USB status
      ShiftRight(Local0 ,7, Local0)  // bit 7 represents AC charger status
      If(LOr(LEqual(Local1, 0x1), LEqual(Local0, 0x1)))
      {
        Return(1)  //USB or AC charger presents or both present.
      }
      else
      {
        Return(0)
      }
    }

    Method(_STA, 0)
    {
      If(LEqual(BCSL , 0))                                // Battery charging solution is non-ULPMC by reading GlobalNvs BCSL
      {
        Return(0x000F)
      }
      Return(0x0)
    }

    // Return that everything runs off of AC.
    Method(_PCL,0)
    {
      Return(\_SB)
    }
  } //Device(ADP2)
}