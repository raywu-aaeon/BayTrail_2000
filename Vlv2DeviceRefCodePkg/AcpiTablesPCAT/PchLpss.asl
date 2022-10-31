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
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
//
// LPIO1 DMA#1 (Synopsis GP DMA)
//
Device (GDM1)
{
  Name (_HID, "INTL9C60")
  //Name (_CID, "INTL9C60")
  //Name (_CLS, Package (3) {0x08, 0x01, 0x02})
  //Name (_HID, "INTL0005")
  Name (_DDN, "Intel(R) DMA Controller #1 - INTL9C60")
  Name (_UID, 1)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {42}  // DMA #1 IRQ
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(D10A, B0BA)
    Store(D10L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(D11A, B1BA)
    //Store(D11L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(D10A, 0), LEqual(L10D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
}

//
// LPIO1 DMA#2 (Synopsis GP DMA)
//
Device (GDM2)
{
  Name (_HID, "INTL9C60")
  // Name (_CID, "INTL9C60")
  //Name (_CLS, Package (3) {0x08, 0x01, 0x02})
  //Name (_HID, "INTL0005")
  Name (_DDN, "Intel(R) DMA Controller #2 - INTL9C60")
  Name (_UID, 2)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {43}  // DMA #2 IRQ
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(D20A, B0BA)
    Store(D20L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(D21A, B1BA)
    //Store(D21L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(D20A, 0), LEqual(L20D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
}

//
// LPIO1 PWM #1
//
Device(PWM1)
{
  Name (_ADR, 0)
  Name (_HID, "80860F09")
  Name (_CID, "80860F09")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) PWM Controller #1 - 80860F08")
  Name (_UID, 1)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(P10A, B0BA)
    Store(P10L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(P11A, B1BA)
    //Store(P11L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(P10A, 0), LEqual(L11D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
}

//
// LPIO1 PWM #2
//
Device(PWM2)
{
  Name (_ADR, 0)
  Name (_HID, "80860F09")
  Name (_CID, "80860F09")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) PWM Controller #2 - 80860F09")
  Name (_UID, 2)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(P20A, B0BA)
    Store(P20L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(P21A, B1BA)
    //Store(P21L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(P20A, 0), LEqual(L12D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
}

//
// LPIO1 HS-UART #1
//
Device(URT1)
{
  Name (_ADR, 0)
  Name (_HID, "80860F0A")
  Name (_CID, "80860F0A")
  //Name (_CLS, Package (3) {0x07, 0x80, 0x00})
  Name (_DDN, "Intel(R) HS-UART Controller #1 - 80860F0A")
  Name (_UID, 1)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {39}  // HS-UART #1 IRQ

    FixedDMA(0x2, 0x2, Width32Bit, )
    FixedDMA(0x3, 0x3, Width32Bit, )
  })
  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(U10A, B0BA)
    Store(U10L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(U11A, B1BA)
    //Store(U11L, B1LN)
    Return (RBUF)
  }

  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(U10A, 0), LEqual(L13D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, U11A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

  //
  // Bluetooth controller using serial interface
  //
  Device(BTH0)
  {
    Name(_HID, "BCM2E1A")      // BCM2E1A Vendor BRCM request to change to 2E1A for BYT, #4753211
    Method(_STA, 0x0, NotSerialized)
    {
      If (LEqual(WITT, 1)) {
        Return(0)
      }
      Return(0x0F)
    } //_STA
    Method(_CRS, 0x0, Serialized)
    {
      Name(UBUF, ResourceTemplate ()
      {
        // UARTSerial Bus Connection Descriptor
        UARTSerialBus(115200,   // InitialBaudRate: in bits ber second
                      ,             // BitsPerByte: default to 8 bits
                      ,             // StopBits: Defaults to one bit
                      0xfc,                   // LinesInUse: 8 1-bit flags to declare line enabled
                      ,             // IsBigEndian: default to LittleEndian
                      ,             // Parity: Defaults to no parity
                      ,             // FlowControl: Defaults to no flow control
                      32,             // ReceiveBufferSize
                      32,             // TransmitBufferSize
                      "\\_SB.URT1",       // ResourceSource: UART bus controller name
                      ,)            // DescriptorName: creates name for offset of resource descriptor
        // OUT pin, BT_EN pin
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {70}
        //GpioInt(Edge, ActiveHigh, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {17}        //  COMBO_UART_WAKE_R 128+17
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {35}  //   COMBO_BT_WAKEUP
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {97}  // - COMBO_BT_RESET_N
      })
      Name(PBUF, ResourceTemplate ()
      {
        // UARTSerial Bus Connection Descriptor
        UARTSerialBus(115200,   // InitialBaudRate: in bits ber second
                      ,             // BitsPerByte: default to 8 bits
                      ,             // StopBits: Defaults to one bit
                      0xfc,                   // LinesInUse: 8 1-bit flags to declare line enabled
                      ,             // IsBigEndian: default to LittleEndian
                      ,             // Parity: Defaults to no parity
                      ,             // FlowControl: Defaults to no flow control
                      32,             // ReceiveBufferSize
                      32,             // TransmitBufferSize
                      "\\_SB.URT1",       // ResourceSource: UART bus controller name
                      ,)            // DescriptorName: creates name for offset of resource descriptor
        // OUT pin, BT_EN pin
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {70}
        //GpioInt(Edge, ActiveHigh, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {17}        //  COMBO_UART_WAKE_R 128+17
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {97}  // - COMBO_BT_RESET_N
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {35}  //   COMBO_BT_WAKEUP
      })
      Name(RBUF, ResourceTemplate ()
      {
        // UARTSerial Bus Connection Descriptor for Android
        UARTSerialBus(115200,   // InitialBaudRate: in bits ber second
                      ,             // BitsPerByte: default to 8 bits
                      ,             // StopBits: Defaults to one bit
                      0xfc,                   // LinesInUse: 8 1-bit flags to declare line enabled
                      ,             // IsBigEndian: default to LittleEndian
                      ,             // Parity: Defaults to no parity
                      ,             // FlowControl: Defaults to no flow control
                      32,             // ReceiveBufferSize
                      32,             // TransmitBufferSize
                      "\\_SB.URT1",       // ResourceSource: UART bus controller name
                      ,)            // DescriptorName: creates name for offset of resource descriptor
        // OUT pin, BT_EN pin
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {70}
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {35}  //   COMBO_BT_WAKEUP
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {97}  // - COMBO_BT_RESET_N
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionInputOnly, "\\_SB.GPO2", ) {27}   //   UART_WAKE_R
      })
      // If OSEL = 1, return ResourceTemplate RBUF for Android
      if (Lequal(OSEL, 1))
      {
        return(RBUF)
      }
            if (Lequal(BDID,0x4))
      {
        if(Lequal(FBID,0x0))
        {
          return(PBUF)//PR0
        }
      }
            return(UBUF)
    }
  } // Device BTH0

}//  Device (URT1)

//
// LPIO1 HS-UART #2
//
Device(URT2)
{
  Name (_ADR, 0)
  Name (_HID, "80860F0A")
  Name (_CID, "80860F0A")
  //Name (_CLS, Package (3) {0x07, 0x80, 0x00})
  Name (_DDN, "Intel(R) HS-UART Controller #2 - 80860F0C")
  Name (_UID, 2)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {40}  // HS-UART #2 IRQ

    FixedDMA(0x4, 0x4, Width32Bit, )
    FixedDMA(0x5, 0x5, Width32Bit, )
  })
  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(U20A, B0BA)
    Store(U20L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(U21A, B1BA)
    //Store(U21L, B1LN)
    Return (RBUF)
  }

  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(U20A, 0), LEqual(L14D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, U21A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

         Device(GPS0)
  {
    Name(_HID, "BCM4752")      //Vendor: test GPS device for BYT
    Name(_HRV, 0)
    Method(_CRS, 0x0, Serialized)
    {
      // UARTSerial Bus Connection Descriptor

      Name(UBUF, ResourceTemplate ()
      {
        UARTSerialBus(
          115200,   // InitialBaudRate: in bits ber second
          ,             // BitsPerByte: default to 8 bits
          ,             // StopBits: Defaults to one bit
          0xfc,                   // LinesInUse: 8 1-bit flags to declare line enabled
          ,             // IsBigEndian: default to LittleEndian
          ,             // Parity: Defaults to no parity
          FlowControlHardware,             // FlowControl: Defaults to no flow control
          32,             // ReceiveBufferSize
          32,             // TransmitBufferSize
          "\\_SB.URT2",       // ResourceSource: UART bus controller name
          ,)            // DescriptorName: creates name for offset of resource descriptor
        //
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {98}      // COMBO_GPS_RESET_N
        //GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {100}       // GPS_R_WAKEUP
      })

      Return (UBUF)
    }
  } // Device GPS0

}//  Device (URT2)

//
// LPIO1 SPI
//
Device(SPI1)
{
  Name (_ADR, 0)
  Name (_HID, "80860F0E")
  Name (_CID, "80860F0E")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (_DDN, "Intel(R) SPI Controller - 80860F0E")
  //Name (_UID, 1)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {41}  // SPI IRQ

    FixedDMA(0x0, 0x0, Width32Bit, )
    FixedDMA(0x1, 0x1, Width32Bit, )
  })
  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(SP0A, B0BA)
    Store(SP0L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(SP1A, B1BA)
    //Store(SP1L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(SP0A, 0), LEqual(L15D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, SP1A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

         Device(FPNT)
  {
    Name(_HID, "AUTH2750")      // AuthenTec AES2750
    Name(_DDN, "AuthenTec AES2750")
    Method(_CRS, 0x0, Serialized)
    {
      // SpiSerial Bus Connection Descriptor
      Name(UBUF, ResourceTemplate ()
      {
        SPISerialBus(
          0,                 // Device selection
          PolarityLow,                       // Device selection polarity
          FourWireMode,                       // wiremode
          8,                   // databit len
          ControllerInitiated,                       // slave mode
          8000000,                       // Connection speed
          ClockPolarityLow,                       // Clock polarity
          ClockPhaseSecond,                     // clock phase
          "\\_SB.SPI1",           // ResourceSource: SPI bus controller name
          0,                      // ResourceSourceIndex
          ResourceConsumer,                       // Resource usage
          ,                       // DescriptorName: creates name for offset of resource descriptor
        )                      // Vendor Data
        Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, , , ) {72}
        //GpioInt(Edge, ActiveHigh, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {1}
      })
      Return (UBUF)
    }
    Method (_STA, 0x0, NotSerialized)
    {
      Return (0xF)
    }

  }// Device(FPNT)

}//  Device (SPI1)

//
// LPIO2 I2C #1
//
Device(I2C1)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (_DDN, "Intel(R) I2C Controller #1 - 80860F41")
  Name (_UID, 1)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {32}  // I2C #1 IRQ

    FixedDMA(0x10, 0x0, Width32Bit, )
    FixedDMA(0x11, 0x1, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I10A, B0BA)
    Store(I10L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I11A, B1BA)
    //Store(I11L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I10A, 0), LEqual(L21D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)

  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I11A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

}

//
// LPIO2 I2C #2
//
Device(I2C2)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (_DDN, "Intel(R) I2C Controller #2 - 80860F42")
  Name (_UID, 2)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {33}  // I2C #2 IRQ

    FixedDMA(0x12, 0x2, Width32Bit, )
    FixedDMA(0x13, 0x3, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, Serialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I20A, B0BA)
    Store(I20L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I21A, B1BA)
    //Store(I21L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I20A, 0), LEqual(L22D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)

  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I21A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }


  //
  // Realtek Audio Codec
  //
  Device (RTEK)   //Audio Codec driver I2C
  {
    Name (_ADR, 0)
    Name (_HID, "10EC5640")
    Name (_CID, "10EC5640")
    Name (_DDN, "RTEK Codec Controller " )
    Name (_UID, 1)


    Method(_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        I2CSerialBus(0x1C,          //SlaveAddress: bus address
                     ,                         //SlaveMode: default to ControllerInitiated
                     400000,                   //ConnectionSpeed: in Hz
                     ,                         //Addressing Mode: default to 7 bit
                     "\\_SB.I2C2",             //ResourceSource: I2C bus controller name
                     ,                         //ResourceSourceIndex: defaults to 0
                     ,                         //ResourceUsage: Defaults to ResourceConsumer
                     ,                         //Descriptor Name: creates name for offset of resource descriptor
                    )  //VendorData
        GpioInt(Edge, ActiveHigh, ExclusiveAndWake, PullNone, 0,"\\_SB.GPO2") {4} //  AUD_INT
      })
      Return (SBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {

        If (LEqual(LPEE, 2)) { // LPE Audio ACPI Mode
        Return(0xF)
      }
      Return(0)
    }

    Method (_DIS, 0x0, NotSerialized)
    {

    }
  } // Device (RTEK)
} //  Device (I2C2)

//
// LPIO2 I2C #3
//
Device(I2C3)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) I2C Controller #3 - 80860F43")
  Name (_UID, 3)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {34}  // I2C #3 IRQ

    FixedDMA(0x14, 0x4, Width32Bit, )
    FixedDMA(0x15, 0x5, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I30A, B0BA)
    Store(I30L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I31A, B1BA)
    //Store(I31L, B1LN)
    Return (RBUF)
  }

  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I30A, 0), LEqual(L23D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)

  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I31A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
    PSAT,   32
  }


}

//
// LPIO2 I2C #4
//
Device(I2C4)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  // Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) I2C Controller #4 - 80860F44")
  Name (_UID, 4)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {35}  // I2C #4 IRQ

    FixedDMA(0x16, 0x6, Width32Bit, )
    FixedDMA(0x17, 0x7, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }


  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I40A, B0BA)
    Store(I40L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I41A, B1BA)
    //Store(I41L, B1LN)
    Return (RBUF)
  }

  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I40A, 0), LEqual(L24D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)

  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I41A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
    PSAT,   32
  }

  //W/A for 9385 RTD3 flow for CAM
  PowerResource (CLK0, 0x00, 0x0000)
  {
    Method (_STA, 0, NotSerialized)   // _STA: Status
    {
      //P8XH(0,0x33) // Output 66 to  postcode on Port 80h.
      Return (CKC0)
    }

    Method (_ON, 0, NotSerialized)   // _ON_: Power On
    {
      //P8XH(0,0x17) // Output 66 to  postcode on Port 80h.
      Store (One, CKC0)
      Store (One, CKF0)
      Sleep (0x20)
      //P8XH(0,0x66) // Output 66 to  postcode on Port 80h.
    }

    Method (_OFF, 0, NotSerialized)   // _OFF: Power Off
    {
      Store (0x02, CKC0)
      //P8XH(0,0x55) // Output 66 to  postcode on Port 80h.
    }
  }
  PowerResource (CLK1, 0x00, 0x0000)
  {
    Method (_STA, 0, NotSerialized)   // _STA: Status
    {
      //P8XH(0,0x22) // Output 66 to  postcode on Port 80h.
      Return (CKC1)
    }

    Method (_ON, 0, NotSerialized)   // _ON_: Power On
    {
      //P8XH(0,0x27) // Output 66 to  postcode on Port 80h.
      Store (One, CKC1)
      Store (One, CKF1)
      Sleep (0x20)
      //P8XH(0,0x88) // Output 66 to  postcode on Port 80h.
    }

    Method (_OFF, 0, NotSerialized)   // _OFF: Power Off
    {
      Store (0x02, CKC1)
      //P8XH(0,0x77) // Output 66 to  postcode on Port 80h.
    }
  }


  ///Device CAM0---Front Camera
  Device (CAM0)
  {
    Name(_ADR, 0x00)
    Name(_HID, "INTCF0B")
    Name(_CID, "INTCF0B")
    Name(_SUB, "INTL0000")
    Name(_DDN, "OV2720")
    Name(_UID, 0x01)
    //Return the PowerResource of D0 & D3, used for OS
    //Name(_PR0,Package(){P28X, P18X})
    Name(_PR0,Package() {CLK1})

    Name (PLDB, Package(1)
    {
      Buffer(0x14)
      {
        0x82,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x61,0x0c,0x00,0x00,
        0x03,0x00,0x00,0x00,
        0xFF,0xFF,0xFF,0xFF
      }
    })

    Method(_PLD,0,Serialized)
    {
      Return (PLDB)
    }

      Method (_STA, 0, NotSerialized) {
  		  if (Lequal(BDID,0x4)){
  			  if(Lequal(FBID,0x2)){		
          return(0x00)//PR1
        }
      }
            return(0x0F)
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {22}      //MUX_CAM2_PWRDWN
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {25}    //CAM_2_RST_N
        I2CSerialBus(0x36,                 //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
      })
      Return (SBUF)
    }
    /*
    Method (_PS3, 0, NotSerialized)
    {
      Store(2, CKC1)    //force off
    }

    Method (_PS0, 0, NotSerialized)
    {
      Store(1, CKC1)    //force on
      Store(1, CKF1)   //19.2M
      sleep (5)
    }*/
    Method(_DSM, 0x4, NotSerialized)
    {
      If(LEqual(Arg0, ToUUID("377BA76A-F390-4AFF-AB38-9B1BF33A3015")))   //HWID
      {
        Return("INTCF0B");
      }
      If(LEqual(Arg0, ToUUID("3C62AAAA-D8E0-401A-84C3-FC05656FA28C")))   //Sensor CMOS Name
      {
        Return("OV2720")
      }
      If(LEqual(Arg0, ToUUID("822ACE8F-2814-4174-A56B-5F029FE079EE")))   //module name
      {
        Return("11P2SF208")
      }
      If(LEqual(Arg0, ToUUID("2959512A-028C-4646-B73D-4D1B5672FAD8")))   //Customer/platform info string
      {
        Return("INTEL_RVP")
      }
      If(LEqual(Arg0, ToUUID("918AB242-C37C-450A-9D0F-F47AB97C3DEA")))   //MIPI lanes count
      {
        Return(0x0101) // 1 lanes
      }
      If(LEqual(Arg0, ToUUID("EA3B7BD8-E09B-4239-AD6E-ED525F3F26AB")))   //MIPI Port
      {
        Return(0x00)  // csi portx1
      }
      If(LEqual(Arg0, ToUUID("B65AC492-9E30-4D60-B5B2-F497C790D9CF")))   //DIR
      {
        Return(0x0)  // degree 0
      }
      If(LEqual(Arg0, ToUUID("E770AB0F-2644-4BAB-8628-D62F1683FB9D")))   //ROM
      {
        Return(0x0)  // none
      }
      If(LEqual(Arg0, ToUUID("1EA54AB2-CD84-48CC-9DD4-7F594EC3B015")))   // old power option
      {
        Return(0x00)
      }
      If(LEqual(Arg0, ToUUID("8DBE2651-70C1-4C6F-AC87-A37CB46E4AF6")))   // old mclk option
      {
        Return(0x01)
      }
      If(LEqual(Arg0, ToUUID("75C9A639-5C8A-4A00-9F48-A9C3B5DA789F")))   //Reserved, default return 0x0
      {
        Return(0x0)
      }
      If(LEqual(Arg0, ToUUID("26257549-9271-4CA4-BB43-C4899D5A4881")))   //I2c
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x1);
        }
        if(LEqual(Arg2, 2))   // Function 1 - general
        {
          Return(0x04003600);
        }
      }
      If(LEqual(Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F")))   //GPIO
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x2);
        }
        if(LEqual(Arg2, 2))   // Function 1 - RST
        {
          Return(0x01001600);
        }
        if(LEqual(Arg2, 3))   // Function 2 - PWDN
        {
          Return(0x01001901);
        }
      }
      Return(0x00)
    }

  } //END CAM0

  ///Device CAM1-----Back Camera
  Device (CAM1)
  {
    Name(_ADR, 0x00)
    Name(_HID, "INTCF1A")
    Name(_CID, "INTCF1A")
    Name(_SUB, "INTL0000")
    Name(_DDN, "Sony IMX175")
    Name(_UID, 0x01)
    //Return the PowerResource of D0 & D3, used for OS
    //Name(_PR0,Package(){P28X, P18X})
    Name(_PR0,Package() {CLK0})

    Name (PLDB, Package(1)
    {
      Buffer(0x14)
      {
        0x82,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x69,0x0c,0x00,0x00,
        0x03,0x00,0x00,0x00,
        0xFF,0xFF,0xFF,0xFF
      }
    })

    Method(_PLD,0,Serialized)
    {
      Return (PLDB)
    }

      Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        //GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {21}       //MUX_CAM1_PWRDWN
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {24}     //CAM_1_RST_N
        //GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {15}       //MUX_CAM_ACT_LED

        I2CSerialBus(0x10,                 //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
        I2CSerialBus(0xC,                  //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
        I2CSerialBus(0x54,                 //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
      })
      Return (SBUF)
    }
    /*
    Method (_PS3, 0, NotSerialized)
    {
      Store(2, CKC0)    //force off
    }

    Method (_PS0, 0, NotSerialized)
    {
      Store(1, CKC0)    //force on
      Store(1, CKF0)   //19.2M
      sleep (5)
    }*/
    Method(_DSM, 0x4, NotSerialized)
    {
      If(LEqual(Arg0, ToUUID("377BA76A-F390-4AFF-AB38-9B1BF33A3015")))   //HWID
      {
        Return("INTCF1A");
      }
      If(LEqual(Arg0, ToUUID("3C62AAAA-D8E0-401A-84C3-FC05656FA28C")))   //Sensor CMOS Name
      {
        Return("IMX175")
      }
      If(LEqual(Arg0, ToUUID("822ACE8F-2814-4174-A56B-5F029FE079EE")))   //module name
      {
        Return("SS89A849")
      }
      If(LEqual(Arg0, ToUUID("2959512A-028C-4646-B73D-4D1B5672FAD8")))   //Customer/platform info string
      {
        Return("INTEL_RVP")
      }
      If(LEqual(Arg0, ToUUID("918AB242-C37C-450A-9D0F-F47AB97C3DEA")))   //MIPI lanes count
      {
        Return(0x0104) // 4 lanes
      }
      If(LEqual(Arg0, ToUUID("EA3B7BD8-E09B-4239-AD6E-ED525F3F26AB")))   //MIPI Port
      {
        Return(0x01)  // csi portx4
      }
      If(LEqual(Arg0, ToUUID("B65AC492-9E30-4D60-B5B2-F497C790D9CF")))   //DIR
      {
        Return(0x0)  // degree 0
      }
      If(LEqual(Arg0, ToUUID("E770AB0F-2644-4BAB-8628-D62F1683FB9D")))   //ROM
      {
        Return(0x2)  // eeprom
      }
      If(LEqual(Arg0, ToUUID("1EA54AB2-CD84-48CC-9DD4-7F594EC3B015")))   // old power option
      {
        Return(0x00)
      }
      If(LEqual(Arg0, ToUUID("8DBE2651-70C1-4C6F-AC87-A37CB46E4AF6")))   // old mclk option
      {
        Return(0x00)
      }
      If(LEqual(Arg0, ToUUID("75C9A639-5C8A-4A00-9F48-A9C3B5DA789F")))   //Reserved, default return 0x0
      {
        Return(0x0)
      }
      If(LEqual(Arg0, ToUUID("26257549-9271-4CA4-BB43-C4899D5A4881")))   //I2c
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x3);
        }
        if(LEqual(Arg2, 2))   // Function 1 - general
        {
          Return(0x04001000);
        }
        if(LEqual(Arg2, 3))   // Function 2 - vcm
        {
          Return(0x04000c01);
        }
        if(LEqual(Arg2, 4))   // Function 3 - eeprom
        {
          Return(0x04005402);
        }
      }
      If(LEqual(Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F")))   //GPIO
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x1);
        }
        if(LEqual(Arg2, 2))   // Function 1 - RST
        {
          Return(0x01001800);
        }
      }
      Return(0x00)
    }
  }  //END CAM1

  ///Device CAM2
  Device (CAM2)
  {
    Name(_ADR, 0x00)
    Name(_HID, "INT33FB")
    Name(_CID, "INT33FB")
    Name(_SUB, "INTL0000")
    Name(_DDN, "OV2722")
    Name(_UID, 0x01)
    //Return the PowerResource of D0 & D3, used for OS
    //Name(_PR0,Package(){P28X, P18X})
    Name(_PR0,Package() {CLK1})

    Name (PLDB, Package(1)
    {
      Buffer(0x14)
      {
        0x82,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x61,0x0c,0x00,0x00,
        0x03,0x00,0x00,0x00,
        0xFF,0xFF,0xFF,0xFF
      }
    })

    Method(_PLD,0,Serialized)
    {
      Return (PLDB)
    }

    Method (_STA, 0, NotSerialized)
    {

      if (Lequal(BDID,0x4))
      {
        if(Lequal(FBID,0x2))
        {
          return(0x0F)//PR1
        }
      }
      return(0x00)
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {22}      //MUX_CAM2_PWRDWN
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {25}    //CAM_2_RST_N
        I2CSerialBus(0x36,                 //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
      })
      Return (SBUF)
    }
    /*
    Method (_PS3, 0, NotSerialized)
    {
      Store(2, CKC1)    //force off
    }

    Method (_PS0, 0, NotSerialized)
    {
      Store(1, CKC1)    //force on
      Store(1, CKF1)   //19.2M
      sleep (5)
    }*/
    Method(_DSM, 0x4, NotSerialized)
    {
      If(LEqual(Arg0, ToUUID("377BA76A-F390-4AFF-AB38-9B1BF33A3015")))   //HWID
      {
        Return("INT33FB");
      }
      If(LEqual(Arg0, ToUUID("3C62AAAA-D8E0-401A-84C3-FC05656FA28C")))   //Sensor CMOS Name
      {
        Return("OV2722")
      }
      If(LEqual(Arg0, ToUUID("822ACE8F-2814-4174-A56B-5F029FE079EE")))   //module name
      {
        Return("12P2SF220")
      }
      If(LEqual(Arg0, ToUUID("2959512A-028C-4646-B73D-4D1B5672FAD8")))   //Customer/platform info string
      {
        Return("INTEL_RVP")
      }
      If(LEqual(Arg0, ToUUID("918AB242-C37C-450A-9D0F-F47AB97C3DEA")))   //MIPI lanes count
      {
        Return(0x0101) // 1 lanes
      }
      If(LEqual(Arg0, ToUUID("EA3B7BD8-E09B-4239-AD6E-ED525F3F26AB")))   //MIPI Port
      {
        Return(0x00)  // csi portx1
      }
      If(LEqual(Arg0, ToUUID("B65AC492-9E30-4D60-B5B2-F497C790D9CF")))   //DIR
      {
        Return(0x0)  // degree 0
      }
      If(LEqual(Arg0, ToUUID("E770AB0F-2644-4BAB-8628-D62F1683FB9D")))   //ROM
      {
        Return(0x0)  // none
      }
      If(LEqual(Arg0, ToUUID("1EA54AB2-CD84-48CC-9DD4-7F594EC3B015")))   // old power option
      {
        Return(0x00)
      }
      If(LEqual(Arg0, ToUUID("8DBE2651-70C1-4C6F-AC87-A37CB46E4AF6")))   // old mclk option
      {
        Return(0x01)
      }
      If(LEqual(Arg0, ToUUID("75C9A639-5C8A-4A00-9F48-A9C3B5DA789F")))   //Reserved, default return 0x0
      {
        Return(0x0)
      }
      If(LEqual(Arg0, ToUUID("26257549-9271-4CA4-BB43-C4899D5A4881")))   //I2c
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x1);
        }
        if(LEqual(Arg2, 2))   // Function 1 - general
        {
          Return(0x04003600);
        }
      }
      If(LEqual(Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F")))   //GPIO
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x2);
        }
        if(LEqual(Arg2, 2))   // Function 1 - RST
        {
          Return(0x01001600);
        }
        if(LEqual(Arg2, 3))   // Function 2 - PWDN
        {
          Return(0x01001901);
        }
      }
      Return(0x00)
    }

  } //END CAM0


  //Device STRA
  Device (STRA)
  {
    Name(_ADR, 0x00)
    Name(_HID, "INTCF1C")
    Name(_CID, "INTCF1C")
    Name(_SUB, "INTL0000")
    Name(_DDN, "Flash LM3554")
    Name(_UID, 0x01)
    //Return the PowerResource of D0 & D3, used for OS
    //Name(_PR0,Package(){P28X, P18X})

      Method (_STA, 0, NotSerialized) {
      Return (0x0F)
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {19}       //MUX_FLASH_TRIG
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {20}       //MUX_FLASH_TORCH
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {16}        //MUX_FLASH_RESET_N
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {17}        //MUX_GP_CAMERASB02
        I2CSerialBus(0x53,                 //SlaveAddress: bus address
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C4",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )                       //VendorData
      })
      Return (SBUF)
    }
    Method(_DSM, 0x4, NotSerialized)
    {
      If(LEqual(Arg0, ToUUID("377BA76A-F390-4AFF-AB38-9B1BF33A3015")))   //HWID
      {
        Return("INTCF1C");
      }
      If(LEqual(Arg0, ToUUID("822ACE8F-2814-4174-A56B-5F029FE079EE")))        //module name
      {
        Return("LM3554")
      }
      If(LEqual(Arg0, ToUUID("2959512A-028C-4646-B73D-4D1B5672FAD8")))   //Customer/platform info string
      {
        Return("INTEL_RVP")
      }
      If(LEqual(Arg0, ToUUID("75C9A639-5C8A-4A00-9F48-A9C3B5DA789F")))   //Reserved, default return 0x0
      {
        Return(0x0)
      }
      If(LEqual(Arg0, ToUUID("26257549-9271-4CA4-BB43-C4899D5A4881")))   //I2c
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x1);
        }
        if(LEqual(Arg2, 2))   // Function 1 - general
        {
          Return(0x04005300);
        }
      }
      If(LEqual(Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F")))   //GPIO
      {
        if(LEqual(Arg2, 1))   // Count
        {
          Return(0x4);
        }
        if(LEqual(Arg2, 2))   // Function 1 - Strobe
        {
          Return(0x01001302);
        }
        if(LEqual(Arg2, 3))   // Function 2 - Torch
        {
          Return(0x01001403);
        }
        if(LEqual(Arg2, 4))   // Function 3 - Strobe
        {
          Return(0x01001001);
        }
        if(LEqual(Arg2, 5))   // Function 4 - Torch
        {
          Return(0x01001104);
        }
      }
      Return(0x00)
    }

  } //End STRA
}

//
// LPIO2 I2C #5
//
Device(I2C5)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) I2C Controller #5 - 80860F45")
  Name (_UID, 5)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {36}  // I2C #5 IRQ

    FixedDMA(0x18, 0x0, Width32Bit, )
    FixedDMA(0x19, 0x1, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I50A, B0BA)
    Store(I50L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I51A, B1BA)
    //Store(I51L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I50A, 0), LEqual(L25D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I51A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
    PSAT,   32
  }


  //sensor hub
  Device(SHUB)
  {
    Name(_ADR, 0x0)
    Name(_HID, "SMO91D0")
    Name(_CID, "PNP0C50")
    Name(_UID, 0x1)
    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return(Buffer(One) { 0x03 })
          }
        }
        // Function 1 : HID Function
        If(LEqual(Arg2, One))
        {
          // HID Descriptor Address (IHV Specific)
          Return(0x0001)
        }

        Return(0x00)
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }

    }
    Method(_STA, 0x0, NotSerialized)
    {
       If (LEqual(WITT, 1)) {
        Return(0)
      }
      Return(0x0f)
    }

    Method (_PS3, 0, Serialized)
    {
      if(LEqual (\_SB.GPO0.AVBL, 1))
      {
          Store( 0x00, \_SB.GPO0.SHD3 )   // WL_BL_REQ_ON = 0 puts the device in reset state
        }
    }
    Method (_PS0, 0, Serialized)
    {
      if(LEqual (\_SB.GPO0.AVBL, 1))
      {
          Store( 0x01, \_SB.GPO0.SHD3 )   // WL_BL_REQ_ON = 1 put the device to normal state
        Sleep(50)
      }
    }

    Method(_CRS, 0x0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        I2CSerialBus(0x40,      //SlaveAddress: bus address
                     ,                 //SlaveMode: default to ControllerInitiated
                     400000,           //ConnectionSpeed: in Hz
                     ,                 //Addressing Mode: default to 7 bit
                     "\\_SB.I2C5",     //ResourceSource: I2C bus controller name
                     ,                 //ResourceSourceIndex: defaults to 0
                     ,                 //ResourceUsage: Defaults to ResourceConsumer
                     ,                 //Descriptor Name: creates name for offset of resource descriptor
                    )  //VendorData
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {68}
        //GpioInt(Level, ActiveLow, Exclusive, PullUp, 0, "\\_SB.GPO2", ) {3}//Sensor Hub INT (GPIO INT) SUS 3 + 128
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {95}//RESET
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {59}//WAKE_SH
      })
      Return (SBUF)
    }
  }  // Device SHUB

}

//
// LPIO2 I2C #6
//
Device(I2C6)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) I2C Controller #6 - 80860F46")
  Name (_UID, 6)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {37}  // I2C #6 IRQ

    FixedDMA(0x1A, 0x02, Width32Bit, )
    FixedDMA(0x1B, 0x03, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I60A, B0BA)
    Store(I60L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I61A, B1BA)
    //Store(I61L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(I60A, 0), LEqual(L26D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I61A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

  Device(TCS0)
  {
    Name(_ADR, 0x0)
    Name (_HID, "ATML1000")
    Name (_CID, "PNP0C50")
    Method(_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        I2CSerialBus(0x4A,      //SlaveAddress: bus address 1386->1664
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C6",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )  //VendorData

        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {51}
        //GpioInt(Level, ActiveLow, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {12}  //int Sus12 +128
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {60}//reset score 60
      })
      Return(RBUF)
    }
    Method(_DSM, 0x4, NotSerialized)
    {
      // BreakPoint
      Store ("Method _DSM begin", Debug)

      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // DSM Function
        switch(ToInteger(Arg2))
        {
            // Function 0: Query function, return based on revision
          case(0)
            {
              // DSM Revision
              switch(ToInteger(Arg1))
              {
                  // Revision 1: Function 1 supported
                case(1)
                  {
                    Store ("Method _DSM Function Query", Debug)
                    Return(Buffer(One) { 0x03 })
                  }

                default
                  {
                    // Revision 2+: no functions supported
                    Return(Buffer(One) { 0x00 })
                  }
              }
            }

            // Function 1 : HID Function
          case(1)
            {
              Store ("Method _DSM Function HID", Debug)

              // HID Descriptor Address
              Return(0x0000)
            }

          default
            {
              // Functions 2+: not supported
              Return(0x0000)
            }
        }
      } else
      {
        // No other GUIDs supported
        Return(Buffer(One) { 0x00 })
      }

    }
    Method(_STA, 0x0, NotSerialized)
    {
      if(LEqual(ITSA, 0x4A))
      {
        Return(0xF)
      }
      Return(0x0)
    }
  }//Device(tcs0)

  Device(TCS1)
  {
    Name(_ADR, 0x0)
    Name (_HID, "ATML1000")
    Name (_CID, "PNP0C50")
    Method(_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        I2CSerialBus(0x4B,      //SlaveAddress: bus address 1386->1664
                     ,                       //SlaveMode: default to ControllerInitiated
                     400000,                 //ConnectionSpeed: in Hz
                     ,                       //Addressing Mode: default to 7 bit
                     "\\_SB.I2C6",           //ResourceSource: I2C bus controller name
                     ,                       //ResourceSourceIndex: defaults to 0
                     ,                       //ResourceUsage: Defaults to ResourceConsumer
                     ,                       //Descriptor Name: creates name for offset of resource descriptor
                    )  //VendorData

        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {51}
        //GpioInt(Level, ActiveLow, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {12}  //int Sus12 +128
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {60}//reset score 60
      })
      Return(RBUF)
    }
    Method(_DSM, 0x4, NotSerialized)
    {
      // BreakPoint
      Store ("Method _DSM begin", Debug)

      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // DSM Function
        switch(ToInteger(Arg2))
        {
            // Function 0: Query function, return based on revision
          case(0)
            {
              // DSM Revision
              switch(ToInteger(Arg1))
              {
                  // Revision 1: Function 1 supported
                case(1)
                  {
                    Store ("Method _DSM Function Query", Debug)
                    Return(Buffer(One) { 0x03 })
                  }

                default
                  {
                    // Revision 2+: no functions supported
                    Return(Buffer(One) { 0x00 })
                  }
              }
            }

            // Function 1 : HID Function
          case(1)
            {
              Store ("Method _DSM Function HID", Debug)

              // HID Descriptor Address
              Return(0x0000)
            }

          default
            {
              // Functions 2+: not supported
              Return(0x0000)
            }
        }
      } else
      {
        // No other GUIDs supported
        Return(Buffer(One) { 0x00 })
      }
      Return(0x00)
    }
    Method(_STA, 0x0, NotSerialized)
    {
      if(LEqual(ITSA, 0x4B))
      {
        Return(0xF)
      }
      Return(0x0)
    }
  }//Device(tcs1)
 
 
  //
  // BayTrail-M Touch panel using ATMEL1000
  //
  Device(TCS2)
  {
    Name (_ADR, Zero)
    Name (_HID, "ATML1000")
    Name (_CID, "PNP0C50")
    Name (_UID, One)

    Name (_S0W, 4)          // required to put the device to D3 Cold during S0 idle

    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return(Buffer(One) { 0x03 })
          }
          Else
          {
            Return(Buffer(One) { 0x00 })
          }
        }
        // Function 1 : HID Function
        If(LEqual(Arg2, One))
        {
          // HID Descriptor Address (IHV Specific)
          Return(0x0000)
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
    }

    Method(_STA, 0x0, NotSerialized)
    {
      If(LAnd(LEqual(ITSA, 0x4C), LEqual(OSEL, 0)))
      {
        Return(0xF)
      }
      Return(0)
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name (SBFI, ResourceTemplate ()
      {
        I2cSerialBus (
          0x4C,                  //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          "\\_SB.I2C6",          //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData

        Interrupt(ResourceConsumer, Level, ActiveLow, Exclusive, , ,) {51}
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {60} // reset score 60
      })

      Return (SBFI)
    }
  }//Device(tcs2)

  //
  // BayTrail-M Touch panel using MXT3432
  //
  Device(TCS3)
  {
    Name (_ADR, Zero)
    Name (_HID, "MXT3432")
    Name (_CID, "PNP0C50")

    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return(Buffer(One) { 0x03 })
          }
          Else
          {
            Return(Buffer(One) { 0x00 })
          }
        }
        // Function 1 : HID Function
        If(LEqual(Arg2, One))
        {
          // HID Descriptor Address (IHV Specific)
          Return(0x0000)
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
    }

    Method(_STA, 0x0, NotSerialized)
    {
      If(LAnd(LEqual(ITSA, 0x4C), LEqual(OSEL, 1)))
      {
        Return(0xF)
      }
      Return(0)
    }

    Method (_CRS, 0, Serialized)
    {
      Name (SBFI, ResourceTemplate ()
      {
        I2cSerialBus (
          0x4C,                  //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          "\\_SB.I2C6",          //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData
        Interrupt(ResourceConsumer, Level, ActiveLow, Exclusive, , ,) {51}
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {60} // reset score 60
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {37} //tp power GPIOS_37
      })

      Return (SBFI)
    }
  }//Device(tcs3) for Android

  //
  // Touch panel firmware update
  //
  Device (FWUP)
  {
    Name (_ADR, Zero)
    Name (_HID, "ATML2000")
    Name (_CID, "PNP0C02")
    Name (_UID, 10)

    Method (_STA, 0, NotSerialized)
    {
      If(LEqual(OSEL, 1))
      {
        Return (0)
      }
      Return (0x0F)
    }

    Method (_CRS, 0, Serialized)
    {
      Name (SBFI, ResourceTemplate ()
      {
        I2cSerialBus (
          0x26,                  //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          "\\_SB.I2C6",          //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData

        I2cSerialBus (
          0x27,                  //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          "\\_SB.I2C6",     //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData
      })

      Return (SBFI)
    }
  }  // Device (TPFU)
}

//
// LPIO2 I2C #7
//
Device(I2C7)
{
  Name (_ADR, 0)
  Name (_HID, "80860F41")
  Name (_CID, "80860F41")
  //Name (_CLS, Package (3) {0x0C, 0x80, 0x00})
  Name (_DDN, "Intel(R) I2C Controller #7 - 80860F47")
  Name (_UID, 7)
  Name(_DEP, Package(0x1)
  {
    PEPD
  })
  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {38}  // I2C #7 IRQ

    FixedDMA(0x1C, 0x4, Width32Bit, )
    FixedDMA(0x1D, 0x5, Width32Bit, )
  })

  Method (SSCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x200, 0x200, 0x06 })
    Return (PKG)
  }
  Method (FMCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x55, 0x99, 0x06 })
    Return (PKG)
  }
  Method (FPCN, 0x0, Serialized)
  {
    Name (PKG, Package(3) { 0x1b, 0x3a, 0x06 })
    Return (PKG)
  }

  Method (_HRV, 0x0, NotSerialized)
  {
    Return (SOCS)
  }
  Method (_CRS, 0x0, NotSerialized)
  {
    //P8XH(0,0x51)   // Output 0x51 to Port 80h.
    //sleep(4000)
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(I70A, B0BA)
    Store(I70L, B0LN)
    //CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, ^BAR1._LEN, B1LN)
    //Store(I71A, B1BA)
    //Store(I71L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    //P8XH(0,0x52)   // Output 0x52 to Port 80h.
    //sleep(4000)
    If (LOr(LEqual(I70A, 0), LEqual(L27D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }

  Method (_PS3, 0, NotSerialized)
  {
    OR(PSAT, 0x00000003, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
  }
  OperationRegion (KEYS, SystemMemory, I71A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }

  // NFC Device
  Device (NFC1)
  {
    Name (_ADR, 0x00)
    Name (_HID, "NXP5441")
    Name (_CID, "NXP5441")
    Name (_DDN, "NXP NFC")
    Name (_UID, 0x01)

    Method (_CRS, 0, Serialized)
    {
      Name(SBUF,ResourceTemplate ()
      {
        I2CSerialBus(0x28,                  //SlaveAddress: bus address
                     ,                                   //SlaveMode: default to ControllerInitiated
                     400000,                             //ConnectionSpeed: in Hz
                     ,                                   //Addressing Mode: default to 7 bit
                     "\\_SB.I2C7",                       //ResourceSource: I2C bus controller name
                     ,                                   //ResourceSourceIndex: defaults to 0
                     ,                                   //ResourceUsage: Defaults to ResourceConsumer
                     ,                                   //Descriptor Name: creates name for offset of resource descriptor
                    )                                   //VendorData
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {71}
        //GpioInt(Edge, ActiveHigh, Exclusive, PullNone, 0, "\\_SB.GPO2", ) {10}
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {9} // NFC Reset Pin
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {8} // NFC Enable Pin
      })
      Return (SBUF)
    }

    Method(_STA)
    {
      If (LAnd(LEqual(NFCS, 1), LEqual(NFCE, 1)))
      {
        Return (0xF)
      }
      Return (0x0)
    }
  }// Device (NFC1)

  //------------------------
  //  Synaptics Precision touchpad
  //------------------------
  Device (TPD1)
  {
    Name (_ADR, One)
    Name (_HID, "MSFT0002")
    Name (_CID, "PNP0C50")
    Name (_UID, One)

    Name (_S0W, 3)          // required to put the device to D3 Cold during S0 idle

    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return(Buffer(One) { 0x03 })
          }
          Else
          {
            Return(Buffer(One) { 0x00 })
          }
        }
        // Function 1 : HID Function
        If(LEqual(Arg2, One))
        {
          // HID Descriptor Address (IHV Specific)
          Return(0x0020)
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
    }

    Method (_STA, 0, NotSerialized)
    {
//            If(LEqual(And(SDS1,0x0008), 0x0008)) {
      If (LEqual(TPDE, 1))
      {
        Return (0x0F)
      }
      Else
      {
        Return (0x00)
      }
    }

    Method (_CRS, 0, Serialized)
    {
      Name (SBFI, ResourceTemplate ()
      {
        I2cSerialBus (
          0x20,                  //SlaveAddress: bus address
          ControllerInitiated,   //SlaveMode: Default to ControllerInitiated
          400000,                //ConnectionSpeed: in Hz
          AddressingMode7Bit,    //Addressing Mode: default to 7 bit
          "\\_SB.I2C7",     //ResourceSource: I2C bus controller name
          ,                      //ResourceSourceIndex: defaults to 0
          ,                      //ResourceUsage: Defaults to ResourceConsumer
          ,                      //Descriptor Name: creates name for offset of resource descriptor
        )  //VendorData

        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, , , ) {67}         // DIRECT IRQ
        //GpioInt(Level, ActiveLow, ExclusiveAndWake, PullNone, 0,"\\_SB.GPO2",) {3} // SHARED IRQ
      })

      Return (SBFI)
    }
  }  // Device (TPD1)

}

