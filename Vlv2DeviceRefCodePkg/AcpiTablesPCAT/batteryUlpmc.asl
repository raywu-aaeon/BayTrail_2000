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
  Name(UMPC,ResourceTemplate ()
  {
    I2CSerialBus(0x78,        //SlaveAddress
                 ,                       //SlaveMode: default to ControllerInitiated
                 400000,                 //ConnectionSpeed: in Hz
                 ,                       //Addressing Mode: default to 7 bit
                 "\\_SB.I2C1",           //ResourceSource: I2C bus controller name
                 ,                       //ResourceSourceIndex: defaults to 0
                 ,                       //ResourceUsage: Defaults to ResourceConsumer
                 ,                       //Descriptor Name: creates name for offset of resource descriptor
                )//VendorData
  })

  // Track status of I2C OpRegion availability for this controller
  Name(AVBL, 0)
  Method(_REG,2)
  {
    If (Lequal(Arg0, 9))
    {
      Store(Arg1, ^AVBL)
    }
  }

  OperationRegion(DVUM,GenericSerialBus , 0x00, 0x100) // GenericSerialBus device at command
  Field(DVUM, BufferAcc, NoLock, Preserve)
  {
    Connection(UMPC),                                // Use the Resource Descriptor defined above
    Offset(0x0),
    AccessAs(BufferAcc, AttribBytes(0x53)),          // Use the GenericSerialBus Read/Write Block protocol
    FGC0, 8,
  }

  Field(DVUM, BufferAcc, NoLock, Preserve)
  {
    Connection(UMPC),
    Offset(0x30),
    AccessAs(BufferAcc, AttribBytes(4)),
    FL30, 32, // Virtual register at command value reg0x30. BTP()
  }

  Field(DVUM, BufferAcc, NoLock, Preserve)
  {
    Connection(UMPC),
    Offset(0x4F),
    AccessAs(BufferAcc, AttribByte),
    FL4F, 8, // Virtual register at command value reg0x4F. ULPMC_Interrupt_Status()
    FL50, 8, // Virtual register at command value reg0x50. Battery_Low_threshold()
    FL51, 8, // Virtual register at command value reg0x51. Battery_Critical_threshold()
    FL52, 8, // Virtual register at command value reg0x52. Battery_Reserved_threshold()
    FL53, 8, // Virtual register at command value reg0x53. ULPMC_Command_register()
  }

  Device (BATC)
  {
    Name(_HID, EISAID("PNP0C0A"))
    Name(_UID, 1)
    Name(BCCC, 0x80)   //charging current .  default 0x80=2.500A
    Name(BCCE, 1)      //charging enable flag.  0-disable charging 1-enable charging
    Name(BCCV, 0x37)   //charging voltage.  0x37=4.2V
    Name(BCCL, 0)      //battery cycle

    Name(BCLP, 1200)   //Last fully charged Capacity
    Name(DSCP, 1200)   //Design Capacity
    Name(DSVO, 3750)   //Design Voltage

    Name(BTPC, 0x00)   //Battery Trip point.   0--clear trip point

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

    // Create the GenericSerialBus data buffer for ULPMC
    Name(BFFG, Buffer(0x55) {0xFF}) // 0x53+2
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
    CreateWordField(BFFG, 0x30, CHST) // 8,  ChargerStatusRegister( )
    CreateWordField(BFFG, 0x31, CHCU) // 8,  ChargeCurrentLimit( )
    CreateDWordField(BFFG, 0x32, BTRP)// 32, BatteryTripPoint( )

    CreateByteField(BFFG, 0x36, SKT0) //  8, SkinTemp0( )
    CreateWordField(BFFG, 0x37, ST00) //  16, SkinTemp0Aux0()
    CreateWordField(BFFG, 0x39, ST01) //  16, SkinTemp0Aux1( )
    CreateWordField(BFFG, 0x3B, ST02) //  16, SkinTemp0Aux2( )
    CreateWordField(BFFG, 0x3D, ST03) //  16, SkinTemp0Aux3( )
    CreateByteField(BFFG, 0x3F, SKT1) //  8, SkinTemp1( )
    CreateWordField(BFFG, 0x40, ST10) //  16, SkinTemp1Aux0( )
    CreateWordField(BFFG, 0x42, ST11) //  16, SkinTemp1Aux1( )
    CreateWordField(BFFG, 0x44, ST12) //  16, SkinTemp1Aux2( )
    CreateWordField(BFFG, 0x46, ST13) //  16, SkinTemp1Aux3( )
    CreateByteField(BFFG, 0x48, SKT2) //  8, SkinTemp2( )
    CreateWordField(BFFG, 0x49, ST20) //  16, SkinTemp2Aux0( )
    CreateWordField(BFFG, 0x4B, ST21) //  16, SkinTemp2Aux1( )
    CreateWordField(BFFG, 0x4D, ST22) //  16, SkinTemp2Aux2( )
    CreateWordField(BFFG, 0x4F, ST23) //  16, SkinTemp2Aux3( )

    CreateBytefield(BFFG, 0x51, ULIS) //  8, ULPMC_Interrupt_Status( )
    CreateByteField(BFFG, 0x52, BALT) //  8, Battery_Low_threshold( )
    CreateByteField(BFFG, 0x53, BACT) //  8, Battery_Critical_threshold( )
    CreateByteField(BFFG, 0x54, BART) //  8, Battery_Reserved_threshold( )
    CreateByteField(BFFG, 0x55, ULCR) //  8, ULPMC_Command_register( )

    //CreateByteField(BFFG, 0xF2, BSLM)  //  8, EnterBSLmode( )
    //CreateByteField(BFFG, 0x100, BSCM) //  8, Block Secure Commands( )

    Name(BUFF, Buffer(3) {00,1,00})
    CreateByteField(BUFF, 0x00, BYAT) // offset 0, STAT = Status (Byte)
    // offset 1 is the data size by byte ==1 as always by byte read/write.
    CreateByteField(BUFF, 0x02, DATA) // offset Data = the byte value will write to chipset


    Name(BIXP, Package()    //Data Package for _BIX
    {
      0x00000000,             // Revision, current reversion is 0 according to ACPI5.0
      0x00000001,             // Power Unit 0 = mWh, 1=mAh.
      0xFFFFFFFF,             // Design Capacity.(0xFFFFFFFF - Unknown design capacity.)
      0xFFFFFFFF,             // Last Full Charge.(0xFFFFFFFF - Unknown last full charge capacity)
      0x00000001,             // Secondary Battery Technology.(0x00 - Primary, 0x001 - Secondary)
      0xFFFFFFFF,             // Design Voltage.(0xFFFFFFFF - Unknown design voltage)
      0x0000000A,             // 10% Warning Level.
      0x00000004,             // 4% Low Level.
      0x00000000,             // Cycle Count
      95000,              // Measurement Accuracy //Integer (DWORD) The value 80000 would mean 80% accuracy
      0xFFFFFFFF,             // Max Sampling Time //Integer (DWORD)
      0xFFFFFFFF,             // Min Sampling Time //Integer (DWORD)
      35000,              // Max Averaging Interval //Integer (DWORD)   set to 35000ms
      25000,              // Min Averaging Interval //Integer (DWORD)   set to 25000ms
      0x00000001,             // Battery Capacity Granularity 1 //Integer (DWORD)
      0x00000001,             // Battery Capacity Granularity 2 //Integer (DWORD)
      "SR Real Battery",      // Model Number //String (ASCIIZ)
      "123456789",            // Serial Number //String (ASCIIZ)
      "LION",                 // Battery Type //String (ASCIIZ)
      "Intel SR 1"            // OEM Information //String (ASCIIZ)
    })
    Name(BSTP,Package()
    {
      0x00000000,  // Battery State.
      // Bit0 1 indicates the battery is discharging.
      // Bit1 1 indicates the battery is charging.
      // Bit2 1 indicates the battery is in the critical energy state
      0xFFFFFFFF,  // Battery Present Rate. (in mA)
      0xFFFFFFFF,  // Battery Remaining Capacity. (in mWh)
      0xFFFFFFFF   // Battery Present Voltage. (in mV)
    })

    Method(_BIX,0)
    {
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)          // bytes read ULPMC, 0x39+2 bytes

        If(LNotEqual(STAT, 0x01))
        {
          If (LNotEqual(CYLC,  BCCL ))
          {
            store(CYLC, BCCL)            // update cycle number
            store(FCCT, BCLP)            // update Last fully charged capacity when cycle number changes
          }
          store(BCLP, Index(BIXP,3))       // Last fully charged capacity
          store(CYLC, Index(BIXP,8))       // Cycle count
        }
      }

      Store(DSCP, Index(BIXP,2))           // Design Capacity 1200mAh
      Store(DSVO, Index(BIXP,5))           // Design Voltage  3750mV

      Divide(DSCP, 10, Local0, Local1)     // 10% * Design Capacity
      Store(Local1, Index(BIXP,6))         // Warning Level. 10%
      Divide(DSCP, 50, Local0, Local1)     // 2% * Design Capcity
      Store(Local1, Index(BIXP,7))         // Low Level  2%

      Return (BIXP)
    }

    Method(_BST,0)
    {
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)          // bytes read ULPMC, 0x39+2 bytes

        If(LNotEqual(STAT, 0x01))
        {

          //BSTP.index0-> Bit0 =1 indicates the battery is discharging.
          //BSTP.index0-> Bit1 =1 indicates the battery is charging.
          //BSTP.index0-> Bit2 =1 indicates the battery is in the critical energy state
          ShiftRight(CHST,4,Local0)
          And(Local0, 0x0003, Local0)
          If(Lor(LEqual(Local0, 0x01), LEqual(Local0, 0x02)))     //charging
          {
            Store(0x0002,Local1)
          }

          if (LEqual(local0, 0x00))                       // 'not charging' in CG means discharging
          {
            Store(0x0001,Local1)
          }

          If (LEqual(Local0,0x03))
          {
            Store(0x0000, Local1)                         // charging done by clearing bit2 and bit1
          }
          Store(Local1,Index(BSTP,0))

          Store(AVRC,Local1)
          if(And(Local1,0x8000))                          // check if discharging
          {
            Subtract (0xFFFF, Local1, Local1)
          }  //end if
          Store(Local1,Index(BSTP,1))                     // at rate (MA)

          Store(RECT,Index(BSTP,2))                       // remaining capacity

          Store(VOLT,Index(BSTP,3))
        }
      }
      Return(BSTP)
    } //_BST

    Method(_STA,0)
    {
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)
        If(LNotEqual(STAT, 0x00))    //i2c read failure, ULPMC is not there
        {
          Return(0x0)
        }
        Return(0x001F)
      }
      Return(0x0)
    } //_STA

    Method(_PCL,0)
    {
      Return(\_SB)
    }

    Method(PSOC,0)                                      // Platform state of charge (Remaining Battery Percentage) [DPTF required]
    {
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)
        If(LNotEqual(STAT, 0x01))
        {
          Return(SOCH)
        }
      }
      Return (0)
    }

    Method(PMAX,0)                                      // Maximum platform power that can be supported by the battery in mW. [DPTF required]
    {
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)
        If(LNotEqual(STAT, 0x01))
        {
          Store(MALC,Local1)                              // Maximum Load current
          if(And(Local1,0x8000))                          // check if discharging
          {
            Subtract (0xFFFF, Local1, Local1)
          }  //end if

          MultiPly(DSVO, Local1, Local1)                  // Max load current [mA] * design voltage [mV] = Max Power [mW]
          Return (local1)
        }
      }
      Return (0)
    }

    Method(VMIN,0)                                      // Minimum voltage below which the platform activates OC protection and shuts down in mV [DPTF optional]
    {
      Return (3500)                                     // low battery 3.5V
    }

    Method(APWR,0)                                      // AC Adapter supplied power in mW  [DPTF optional]
    {
      // TODO: Need to confirm with HW the correct value
      // There are adapters supporting from 5V to 19V as per PAD. Current is 3A.   19000mV * 3000mA = 57000000 mW. Hard code maximum capacity.
      Return (57000000)
    }

    Method(NPWR,0)                                      // Could be (AC supplied +Battery pack supplied power) or (AC supplied -power reqd to charge battery) in mW.  [DPTF optional]
    {
      // TODO: Need to confirm with HW the correct value
      Store(APWR(), Local0)                             // AC power
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)
        If(LNotEqual(STAT, 0x01))
        {
          Store(AVRC, Local1)                             // Maximum Load current
          if(And(Local1,0x8000))                          // check if discharging
          {
            Subtract (0xFFFF, Local1, Local1)
          }  //end if

          MultiPly(VOLT, Local1, Local2)                  // voltage * average current
          Subtract(Local0, Local2, Local0)                // AC power - Charging consumed power
          Return (Local0)
        }
      }
      Return (Local0)
    }

    Method(PSRC,0)                                      // System charge source, AC, DC, or USB (for tablets)  [DPTF required]
    {
      // TODO: Need other information to get the power source
      // Temporarily return follows.
      // 0 for none
      // 1 for AC
      // 2 for DC
      // 4 for USB
      if(LEqual (\_SB.I2C1.AVBL, 1))
      {
        Store(\_SB.I2C1.FGC0, BFFG)
        If(LNotEqual(STAT, 0x01))
        {
          Store(CHST, Local0)
          //00:no input          no AC or DC
          //01:USB               usb charger including DCP and CDP
          //10:AC/wall charger   AC adapter only
          //11:OTG               nothing
          ShiftRight(Local0 , 6, Local0)
          And(Local0, 0x0003, Local0)
          If(LEqual(Local0, 0x0))          // none charging source
          {
            Return(0)
          }

          If(LEqual(Local0, 0x2))                                    // AC
          {
            Return(1)
          }

          If(LEqual(Local0, 0x1))                                    // DC   USB charger
          {
            Return(2)
          }

          If(LEqual(Local0, 0x3))                                    // USB  OTG
          {
            Return(4)
          }
        }
      }
      //if i2c fail to read, then ULPMC is not there. There must be a AC.
      Return (1)
    }


  }   //Device(BATC)
}   //  Device (I2C1)


scope(\_SB)
{
  Device(ADP1)    //disable another ADP1 in platform.asl before using this device.
  {
    Name(_HID,"ACPI0003")
    // Return the value that determines if running with AC or not.
    Method(_PSR,0)
    {
      Store(\_SB.I2C1.BATC.PSRC(), Local0)
      If(LEqual(Local0, 1))
      {
        Return(1)           //AC is present
      } Else
      {
        Return(0)
      }
    }

    // Return that everything runs off of AC.
    Method(_PCL,0)
    {
      Return(\_SB)
    }
  } //Device(ADP1)
}
