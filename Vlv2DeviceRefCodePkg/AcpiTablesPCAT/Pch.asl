/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2012 - 2014 Intel Corporation. All rights reserved    *;
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


Scope(\)
{
  //
  // Define VLV ABASE I/O as an ACPI operating region. The base address
  // can be found in Device 31, Registers 40-43h.
  //
  OperationRegion(PMIO, SystemIo, \PMBS, 0x46)
  Field(PMIO, ByteAcc, NoLock, Preserve)
  {
    ,      8,
    PWBS,  1,       // Power Button Status
    Offset(0x20),
    ,      13,
    PMEB,  1,     // PME_B0_STS
    Offset(0x42),     // General Purpose Control
    ,      1,
    GPEC,  1
  }
  Field(PMIO, ByteAcc, NoLock, WriteAsZeros)
  {
    Offset(0x20),     // GPE0 Status
    ,      4,
    PSCI,  1,       // PUNIT SCI Status
    SCIS,  1        // GUNIT SCI Status
  }



  //
  // Define a Memory Region that will allow access to the PMC
  // Register Block.  Note that in the Intel Reference Solution, the PMC
  // will get fixed up dynamically during POST.
  //
  OperationRegion(PMCR, SystemMemory, \PFDR, 0x04)// PMC Function Disable Register
  Field(PMCR,DWordAcc,Lock,Preserve)
  {
    Offset(0x00),   //  Function Disable Register
    L10D,  1,         //  (0) LPIO1 DMA Disable
    L11D,  1,         //  (1) LPIO1 PWM #1 Disable
    L12D,  1,         //  (2) LPIO1 PWM #2 Disable
    L13D,  1,         //  (3) LPIO1 HS-UART #1 Disable
    L14D,  1,         //  (4) LPIO1 HS-UART #2 Disable
    L15D,  1,         //  (5) LPIO1 SPI Disable
    ,          2,     //  (6:7) Reserved
    SD1D,  1,         //  (8) SCC SDIO #1 Disable
    SD2D,  1,         //  (9) SCC SDIO #2 Disable
    SD3D,  1,         //  (10) SCC SDIO #3 Disable
    HSID,  1,         //  (11)
    HDAD,  1,         //  (12) Azalia Disable
    LPED,  1,         //  (13) LPE Disable
    OTGD,  1,         //  (14) USB OTG Disable
    USHD,  1,         //  (15) USH Disable  //AMI_OVERRIDE - CSP20140218_19 fix XHCI auto and smart auto issue
    ,          1,     //  (16)
    ,          1,     //  (17)
    USBD,  1,         //  (18) USB Disable //AMI_OVERRIDE - CSP20140218_19 fix XHCI auto and smart auto issue
    ,          1,     //  (19) SEC Disable
    RP1D,  1,         //  (20) Root Port 0 Disable
    RP2D,  1,         //  (21) Root Port 1 Disable
    RP3D,  1,         //  (22) Root Port 2 Disable
    RP4D,  1,         //  (23) Root Port 3 Disable
    L20D,  1,         //  (24) LPIO2 DMA Disable
    L21D,  1,         //  (25) LPIO2 I2C #1 Disable
    L22D,  1,         //  (26) LPIO2 I2C #2 Disable
    L23D,  1,         //  (27) LPIO2 I2C #3 Disable
    L24D,  1,         //  (28) LPIO2 I2C #4 Disable
    L25D,  1,         //  (29) LPIO2 I2C #5 Disable
    L26D,  1,         //  (30) LPIO2 I2C #6 Disable
    L27D,  1          //  (31) LPIO2 I2C #7 Disable
  }

  // 
  // Support S0, S3, S4, and S5.  The proper bits to be set when 128   // Support S0, S3, S4, and S5.  The proper bits to be set when 
  // entering a given sleep state are found in the Power Management 129   // entering a given sleep state are found in the Power Management 
  // 1 Control ( PM1_CNT ) register, located at ACPIBASE + 04h, 130   // 1 Control ( PM1_CNT ) register, located at PMC Base + 0xA0 
  // bits 10d - 12d.
  // 131   // 
  OperationRegion(PPSC, SystemMemory, Add(\PMCB, 0xA0), 0x08) // PMC Base + 0xA0 
  Field(PPSC,DWordAcc,Lock,Preserve) { 
    DM1P,  1,  //BIT0 
    PW1P,  1,  //BIT1 
    PW2P,  1,  //2 
    UR1P,  1,  //3 
    UR2P,  1,  //4 
    SP1P,  1,  //5 
    SP2P,  1,  //6 
    SP3P,  1,  //7 
    EMMP,  1,  //8 
    SDI1,  1,  //9 
    SDI2,  1,  //10 
    ,      2,  //11-mipi, 12-HDA 
    LPEP,  1,  //13 
    ,      1,  //14 -USB SIP Bridge 
    ,      1,  //15 
    ,      1,  //16 
    SATP,  1,  //17, SATA Power State Current 
    USBP,  1,  //18 
    SECP,  1,  //19, SEC Power State Current 
    PRP1,  1,  //20 
    PRP2,  1,  //21 
    PRP3,  1,  //22 
    PRP4,  1,  //23, PCIE Rootports 1-4 
    DM2P,  1,  //24, LPIO2 DMA 
    IC1P,  1,  //25 
    IC2P,  1,  //26 
    IC3P,  1,  //27 
    IC4P,  1,  //28 
    IC5P,  1,  //29 
    IC6P,  1,  //30 
    IC7P,  1,  //31 
    Offset(0x4), //reg_D3_STS_1_type 
    ,      2,  //0-SMBus 
    ISHP,  1,  //2 
    ,      29 
  }

  OperationRegion(CLKC, SystemMemory, \PCLK, 0x18)// PMC CLK CTL Registers
  Field(CLKC,DWordAcc,Lock,Preserve)
  {
    Offset(0x00),   //  PLT_CLK_CTL_0
    CKC0, 2,
    CKF0, 1,
    ,     29,
    Offset(0x04),   //  PLT_CLK_CTL_1
    CKC1, 2,
    CKF1, 1,
    ,     29,
    Offset(0x08),   //  PLT_CLK_CTL_2
    CKC2,  2,
    CKF2, 1,
    ,     29,
    Offset(0x0C),   //  PLT_CLK_CTL_3
    CKC3,  2,
    CKF3, 1,
    ,     29,
    Offset(0x10),   //  PLT_CLK_CTL_4
    CKC4,  2,
    CKF4, 1,
    ,     29,
    Offset(0x14),   //  PLT_CLK_CTL_5
    CKC5,  2,
    CKF5, 1,
    ,     29,
  }
} //end Scope(\)

scope (\_SB)
{
  Device(LPEA)
  {
    Name (_ADR, 0)
    Name (_HID, "80860F28")
    Name (_CID, "80860F28")
    //Name (_CLS, Package (3) {0x04, 0x01, 0x00})
    Name (_DDN, "Intel(R) Low Power Audio Controller - 80860F28")
    Name (_SUB, "80867270")
    Name (_UID, 1)
#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

    Name(_PR0,Package() {PLPE})

    Method (_STA, 0x0, NotSerialized)
    {
      If (LAnd(LAnd(LEqual(LPEE, 2), LEqual(LPED, 0)), LEqual(OSEL, 0)))
      {
        Return (0xF)
      }
      Return (0x0)
    }

    Method (_DIS, 0x0, NotSerialized)
    {
      //Add a dummy disable function
    }

    Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0xFE400000, 0x00200000, BAR0)  // MMIO 1 - LPE MMIO
        Memory32Fixed (ReadWrite, 0xFE830000, 0x00001000, BAR1)  // MMIO 2 - Shadowed PCI Config Space
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00100000, BAR2)  // LPE Memory Bar Allocate during post
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {24}
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {25}
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {26}
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {27}
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {28}
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {29}
        GpioInt(Edge, ActiveBoth, ExclusiveAndWake, PullNone, 0,"\\_SB.GPO2") {28} //  Audio jack interrupt
      }
    )

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
      Store(LPE0, B0BA)
      CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
      Store(LPE1, B1BA)
      CreateDwordField(^RBUF, ^BAR2._BAS, B2BA)
      Store(LPE2, B2BA)
      Return (RBUF)
    }

    OperationRegion (KEYS, SystemMemory, LPE1, 0x100)
    Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
    {
      Offset (0x84),
      PSAT,   32
    }

    PowerResource(PLPE, 0, 0)   // Power Resource for LPEA
    {
      Method (_STA)
      {
        Return (1)      // Power Resource is always available.
      }

      Method (_ON)
      {
        And(PSAT, 0xfffffffC, PSAT)
        OR(PSAT, 0X00000000, PSAT)
      }

      Method (_OFF)
      {
        OR(PSAT, 0x00000003, PSAT)
        OR(PSAT, 0X00000000, PSAT)
      }
    } // End PLPE
  } // End "Low Power Engine Audio"

  Device(LPA2)
  {
    Name (_ADR, 0)
    Name (_HID, "LPE0F28")  // _HID: Hardware ID
    Name (_CID, "LPE0F28")  // _CID: Compatible ID
    Name (_DDN, "Intel(R) SST Audio - LPE0F28")  // _DDN: DOS Device Name
    Name (_SUB, "80867270")
    Name (_UID, 1)
#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

    Name(_PR0,Package() {PLPE})

    Method (_STA, 0x0, NotSerialized)
    {
      If (LAnd(LAnd(LEqual(LPEE, 2), LEqual(LPED, 0)), LEqual(OSEL, 1)))
      {
        Return (0xF)
      }
      Return (0x0)
    }

    Method (_DIS, 0x0, NotSerialized)
    {
      //Add a dummy disable function
    }

    Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00100000, BAR2)  // LPE Memory Bar Allocate during post
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00000100, SHIM)
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00001000, MBOX)
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00014000, IRAM)
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00028000, DRAM)
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {29}
        Memory32Fixed (ReadWrite, 0xFE830000, 0x00001000, BAR1)  // MMIO 2 - Shadowed PCI Config Space
      }
    )

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^SHIM._BAS, SHBA)
      Add(LPE0, 0x140000, SHBA)
      CreateDwordField(^RBUF, ^MBOX._BAS, MBBA)
      Add(LPE0, 0x144000, MBBA)
      CreateDwordField(^RBUF, ^IRAM._BAS, IRBA)
      Add(LPE0, 0xC0000, IRBA)
      CreateDwordField(^RBUF, ^DRAM._BAS, DRBA)
      Add(LPE0, 0x100000, DRBA)
      CreateDwordField(^RBUF, ^BAR1._BAS, B1BA)
      Store(LPE1, B1BA)
      CreateDwordField(^RBUF, ^BAR2._BAS, B2BA)
      Store(LPE2, B2BA)
      Return (RBUF)
    }

    OperationRegion (KEYS, SystemMemory, LPE1, 0x100)
    Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
    {
      Offset (0x84),
      PSAT,   32
    }

    PowerResource(PLPE, 0, 0)   // Power Resource for LPEA
    {
      Method (_STA)
      {
        Return (1)      // Power Resource is always available.
      }

      Method (_ON)
      {
        And(PSAT, 0xfffffffC, PSAT)
        OR(PSAT, 0X00000000, PSAT)
      }

      Method (_OFF)
      {
        OR(PSAT, 0x00000003, PSAT)
        OR(PSAT, 0X00000000, PSAT)
      }
    } // End PLPE

    Device (ADMA)
    {
      Name (_ADR, Zero)  // _ADR: Address
      Name (_HID, "DMA0F28")  // _HID: Hardware ID
      Name (_CID, "DMA0F28")  // _CID: Compatible ID
      Name (_DDN, "Intel(R) Audio  DMA0 - DMA0F28")  // _DDN: DOS Device Name
      Name (_UID, One)  // _UID: Unique ID
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00001000, DMA0)  // LPE BASE + LPE DMA0 offset
        Memory32Fixed (ReadWrite, 0x55AA55AA, 0x00001000, SHIM)  // LPE BASE + LPE SHIM offset
        Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {24}
      })

      Method (_CRS, 0x0, NotSerialized)   // _CRS: Current Resource Settings
      {
        CreateDwordField(^RBUF, ^DMA0._BAS, D0BA)
        Add(LPE0, 0x98000, D0BA)
        CreateDwordField(^RBUF, ^SHIM._BAS, SHBA)
        Add(LPE0, 0x140000, SHBA)
        Return (RBUF)
      }
    }
  } // End "Low Power Engine Audio" for Android
}

scope (\_SB.PCI0)
{

  //
  // Serial ATA Host Controller - Device 19, Function 0
  //

//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT 
  scope (\_SB.PCI0.SATA) {
#else
  Device(SATA)
  {
    Name(_ADR,0x00130000)
#endif
//AMI_OVERRIDE <<

    //
    // SATA Methods pulled in via SSDT.
    //
    Device(PRT0)
    {
      Name(_ADR,0x0000FFFF)  // Port 0
    }
    Device(PRT1)
    {
      Name(_ADR,0x0001FFFF)  // Port 1
    }
    Device(PRT2)
    {
      Name(_ADR,0x0002FFFF)  // Port 2
    }
    Device(PRT3)
    {
      Name(_ADR,0x0003FFFF)  // Port 3
    }
    
    OperationRegion(SATR, PCI_Config, 0x74,0x4)
    Field(SATR,WordAcc,NoLock,Preserve)
    {
      Offset(0x00), // 0x74, PMCR
      ,   8,
      PMEE,   1,    //PME_EN
      ,   6,
      PMES,   1     //PME_STS
    }

    Method (_STA, 0x0, NotSerialized)
    {

      If(LEqual(OSEL, 2))  
      {
        Sleep(200)
      }

      Return(0xf)
    }

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif  

    Method(_DSW, 3)
    {
    } // End _DSW
  }

  //
  // For eMMC 4.41 PCI mode in order to present non-removable device under Windows environment
  //
  Device(EM41)
  {
    Name(_ADR,0x00100000)
    OperationRegion(SDIO, PCI_Config, 0x84,0x4)
    Field(SDIO,WordAcc,NoLock,Preserve)
    {
      Offset(0x00), // 0x84, PMCR
      ,   8,
      PMEE,   1,    //PME_EN
      ,   6,
      PMES,   1     //PME_STS
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If (LAnd(LEqual(PCIM, 1), LEqual(SD1D, 0)))
      {
        Return(0xF)
      }
      Else
      {
        Return(0x0)
      }
    }

    Method(_DSW, 3)
    {
    } // End _DSW

    Device (CARD)
    {
      Name (_ADR, 0x00000008)
      Method(_RMV, 0x0, NotSerialized)
      {
        Return (0)
      } // End _DSW
    }
  }

  //
  // For eMMC 4.5 PCI mode in order to present non-removable device under Windows environment
  //
  Device(EM45)
  {
    Name(_ADR,0x00170000)
    OperationRegion(SDIO, PCI_Config, 0x84,0x4)
    Field(SDIO,WordAcc,NoLock,Preserve)
    {
      Offset(0x00), // 0x84, PMCR
      ,   8,
      PMEE,   1,    //PME_EN
      ,   6,
      PMES,   1     //PME_STS
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If (LAnd(LEqual(PCIM, 1), LEqual(HSID, 0)))
      {
        Return(0xF)
      }
      Else
      {
        Return(0x0)
      }
    }

    Method(_DSW, 3)
    {
    } // End _DSW

    Device (CARD)
    {
      Name (_ADR, 0x00000008)
      Method(_RMV, 0x0, NotSerialized)
      {
        Return (0)
      } // End _DSW
    }
  }

  // xHCI Controller - Device 20, Function 0
  include("PchXhci.asl")

  //
  // High Definition Audio Controller - Device 27, Function 0
  //
//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT
  scope (\_SB.PCI0.HDEF) {
#else
  Device(HDEF)
  {
    Name(_ADR, 0x001B0000)
#endif
//AMI_OVERRIDE<<
    include("PchAudio.asl")
//    Method(_PRW, 0) { Return(GPRW(0x0D, 4)) }  // can wakeup from S4 state
//    Name(_PRW, Package() {0, 0})

    Method (_STA, 0x0, NotSerialized)
    {
      If (LEqual(HDAD, 0))
      {
        Return(0xf)
      }
      Return(0x0)
    }

    Method(_DSW, 3)
    {
    } // End _DSW
  } // end "High Definition Audio Controller"


#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
  //
  // PCIE Root Port #1
  //
  Device(RP01)
  {
    Name(_ADR, 0x001C0000)
    include("PchPcie.asl")
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
    Name(_PRW, Package() {9, 4})

    Method(_PRT,0)
    {
      If(PICM) { Return(AR04) }// APIC mode
      Return (PR04) // PIC Mode
    } // end _PRT
  } // end "PCIE Root Port #1"

  //
  // PCIE Root Port #2
  //
  Device(RP02)
  {
    Name(_ADR, 0x001C0001)
    include("PchPcie.asl")
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
    Name(_PRW, Package() {9, 4})

    Method(_PRT,0)
    {
      If(PICM) { Return(AR05) }// APIC mode
      Return (PR05) // PIC Mode
    } // end _PRT

  } // end "PCIE Root Port #2"

  //
  // PCIE Root Port #3
  //
  Device(RP03)
  {
    Name(_ADR, 0x001C0002)
    include("PchPcie.asl")
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
    Name(_PRW, Package() {9, 4})
    Method(_PRT,0)
    {
      If(PICM) { Return(AR06) }// APIC mode
      Return (PR06) // PIC Mode
    } // end _PRT

  } // end "PCIE Root Port #3"

  //
  // PCIE Root Port #4
  //
  Device(RP04)
  {
    Name(_ADR, 0x001C0003)
    include("PchPcie.asl")
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
    Name(_PRW, Package() {9, 4})
    Method(_PRT,0)
    {
      If(PICM) { Return(AR07) }// APIC mode
      Return (PR07) // PIC Mode
    } // end _PRT

  } // end "PCIE Root Port #4"
#endif //AMI_OVERRIDE

  Scope(\_SB)
  {
    //
    // Dummy power resource for USB D3 cold support
    //
    PowerResource(USBC, 0, 0)
    {
      Method(_STA) { Return (0xF) }
      Method(_ON) {}
      Method(_OFF) {}
    }
  }
  //
  // EHCI Controller - Device 29, Function 0
  //
//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT 
scope (\_SB.PCI0.EHC1) {
#else
  Device(EHC1)
  {
    Name(_ADR, 0x001D0000)
#endif //AMI_OVERRIDE - for RC0.8.0 
//AMI_OVERRIDE <<

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    
    include("PchEhci.asl")
//    Method(_PRW, 0) { Return(GPRW(0x0D, 4)) }  // can wakeup from S4 state
    Name(_PRW, Package() {0x0D, 4})

    OperationRegion(USBR, SystemMemory, 0xE00E8000,0x60)    
    Field(USBR,WordAcc,NoLock,Preserve)
    {
      Offset(0x04),
      PCMD, 8,
      Offset(0x10),
      UMBA, 32,
      Offset(0x54),
      PSTA, 2,      //Power state
      ,   6,  
      PMEE,   1,    //PME_EN
      ,   6,
      PMES,   1     //PME_STS
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If(LEqual(XHCI, 0))      //XHCI is not present. It means EHCI is there
      {
        Return (0xF)
      } Else
      {
        Return (0x0)
      }
    }
    /*
        Method(_DSW, 3)
        {
        } // End _DSW
    */
    Method (_RMV, 0, NotSerialized)
    {
      Return (0x0)
    }
    //
    // Create a dummy PR3 method to indicate to the PCI driver
    // that the device is capable of D3 cold
    //
    Method(_PR3, 0x0, NotSerialized)
    {
      return (Package() {\_SB.USBC})
    }

  } // end "EHCI Controller"

  //
  // SMBus Controller - Device 31, Function 3
  //
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
  Device(SBUS)
  {
    Name(_ADR,0x001F0003)
    Include("PchSmb.asl")
  }
#endif //AMI_OVERRIDE

  Device(SEC0)
  {
    Name (_ADR, 0x001a0000)                     // Device 0x1a, Function 0
#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    


    OperationRegion (PMEB, PCI_Config, 0x84, 0x04)  //PMECTRLSTATUS
    Field (PMEB, WordAcc, NoLock, Preserve)
    {
      ,   8,
      PMEE,   1,    //bit8 PMEENABLE
      ,   6,
      PMES,   1     //bit15 PMESTATUS
    }

    // Arg0 -- integer that contains the device wake capability control (0-disable 1- enable)
    // Arg1 -- integer that contains target system state (0-4)
    // Arg2 -- integer that contains the target device state
    Method (_DSW, 3, NotSerialized)   // _DSW: Device Sleep Wake
    {
    }

    Method (_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x1f000000, 0x1000000)
      })
      Name (RBUL, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x1f000000, 0x100000)
      })
      If (LEqual(PAVP, 2))
      {
        Return (RBUF)
      }
      Return (RBUL)
    }

    Method (_STA)
    {
      If (LNotEqual(PAVP, 0))
      {
        Return (0xF)
      }
      Return (0x0)
    }
  }   // Device(SEC0)

} // End scope (\_SB.PCI0)
