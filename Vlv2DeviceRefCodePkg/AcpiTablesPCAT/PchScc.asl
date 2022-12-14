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

#ifdef WIN8_SUPPORT     
Device (PEPD)
{
  Name (_HID, "INT3396")
  Name(_CID, 0x800dd041)
  Name (_UID, 0x1)

  // Indicates if the platform PEP has loaded
  Name(PEPP, Zero)

  // Devices score-boarded by the PEP, Rev0 format
  Name (DEVS, Package()
  {
  	1,
	Package() {"\\_SB.PCI0.GFX0"}
  })

  // Devices score-boarded by the PEP, Rev1 format
  Name(DEVX, Package()
  {
    Package () {"\\_SB.PCI0.XHC1", 0xffffffff},
    Package () {"\\_SB.PCI0.EHC1", 0xffffffff},
    Package () {"\\_SB.PCI0.GFX0", 0xffffffff},
    Package () {"\\_SB.PCI0.GFX0.ISP0", 0xffffffff},
    Package () {"\\_SB.PCI0.SEC0", 0xffffffff},
    Package () {"\\_SB.I2C1", 0xffffffff},
    Package () {"\\_SB.I2C2", 0xffffffff},
    Package () {"\\_SB.I2C3", 0xffffffff},
    Package () {"\\_SB.I2C4", 0xffffffff},
    Package () {"\\_SB.I2C5", 0xffffffff},
    Package () {"\\_SB.I2C6", 0xffffffff},
    Package () {"\\_SB.I2C7", 0xffffffff},
    Package () {"\\_SB.SDHA", 0xffffffff},
    Package () {"\\_SB.SDHB", 0xffffffff},
    Package () {"\\_SB.SDHC", 0xffffffff},
    Package () {"\\_SB.SPI1", 0xffffffff},
    Package () {"\\_SB.URT1", 0xffffffff},
    Package () {"\\_SB.URT2", 0xffffffff},
  })
  // Crashdump device package
  Name(CDMP, Package(2) {})
  // Device dependency for uPEP
  Name(DEVY, Package()
  {
      //
      // 1: ACPI Device Descriptor: Fully Qualified namestring
      // 2: Enabled/Disabled Field 
      //      0 = This device is disabled and applies no constraints
      //     >0 = This device is enabled and applies constraints
      // 3: Constraint Package: entry per LPI state in LPIT
      //     a. Associated LPI State UID 
      //         ID == 0xFF: same constraints apply to all states in LPIT
      //     b: minimum Dx state as pre-condition
      //     c: (optional) OEM specific OEM may provide an additional encoding
      //         which further defines the D-state Constraint   
      //            0x0-0x7F - Reserved
      //            0x80-0xFF - OEM defined
      //    
    Package() {"\\_PR.CPU0", 0x1, Package() {0, Package() {0xFF, 0}}},
    Package() {"\\_PR.CPU1", 0x1, Package() {0, Package() {0xFF, 0}}},
    Package() {"\\_PR.CPU2", 0x1, Package() {0, Package() {0xFF, 0}}},
    Package() {"\\_PR.CPU3", 0x1, Package() {0, Package() {0xFF, 0}}},
    Package() {"\\_SB.I2C1", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C2", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C3", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C4", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C5", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C6", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.I2C7", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.PCI0.SATA", 0x1, Package() {0, Package(){0xFF, 3}}},
    Package() {"\\_SB.PCI0.SATA.PRT0", 0x1, Package() {0, Package(){0xFF, 0, 0x81}}},
    Package() {"\\_SB.PCI0.SATA.PRT1", 0x1, Package() {0, Package(){0xFF, 0, 0x81}}},
    Package() {"\\_SB.PCI0.GFX0", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.PCI0.SEC0", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.PCI0.XHC1", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.PCI0.GFX0.ISP0", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.LPEA", 0x1, Package() {0, Package() {0x0, 3}, Package() {0x1, 0}, Package() {0x2, 3}, Package() {0x3, 3}}},
    Package() {"\\_SB.SDHA", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.SDHB", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.SDHC", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.SPI1", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.URT1", 0x1, Package() {0, Package() {0xFF, 3}}},
    Package() {"\\_SB.URT2", 0x1, Package() {0, Package() {0xFF, 3}}}
  })
  // BCCD crashdump information
  Name(BCCD, Package()
  {
      //
      // 1: ACPI Device Descriptor: Fully Qualified name string
      // 2: Package of packages: 1 or more specific commands to power up critical device
      //  2a: Package: GAS-structure describing location of PEP accessible power control
      //    Refer to ACPI 5.0 spec section 5.2.3.1 for details
      //    a: Address Space ID (0 = System Memory)
      //       NOTE: A GAS Address Space of 0x7F (FFH) indicates remaining package
      //             elements are Intel defined
      //    b: Register bit width (32 = DWORD)
      //    c: Register bit offset
      //    d: Access size (3 = DWORD Access)
      //    e: Address (for System Memory = 64-bit physical address)
      //  2b: Package containing:
      //    a: AND mask !V not applicable for all Trigger Types
      //    b: Value (bits required to power up the critical device)
      //    c: Trigger Type:
      //         0 = Read
      //         1 = Write
      //         2 = Write followed by Read
      //         3 = Read Modify Write
      //         4 = Read Modify Write followed by Read
      //  2c: Power up delay: Time delay before next operation in uSec
      //
     Package() {"\\_SB.PCI0.SATA", Package(){
       Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                  Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                  16000}                        // Power up delay = 16ms
       }
     },
     Package() {"\\_SB.PCI0.SAT0.PRT0", Package(){
       Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                  Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                  16000}                        // Power up delay = 16ms
       }
     },
     Package() {"\\_SB.PCI0.SAT0.PRT1", Package(){
       Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                  Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                  16000}                        // Power up delay = 16ms
       }
     },
     Package() {"\\_SB.SDHA", Package(){
       Package() {Package() {0, 32, 0,  3, 0xFFFFFFFFFFFFFFFF},
                  Package() {0xFFFFFFFC, 0x0, 0x4}, 
                  0}
       }
     }
  })

  Method(_STA, 0x0, NotSerialized)
  {
     //Check if OS is post-Win8.1, the actual OS identifier may be different
      If(LGreaterEqual(OSYS,2015))  
      {
          Return(0xf)
      }	
      If(LGreaterEqual(OSYS,2012))
      {
         If(LEqual(S0IX, 1)) {
           Return(0xf)
        }
      }

      Return (0x0)
  }

  Method(_DSM, 0x4, Serialized)
  {
    If(LEqual(Arg0,ToUUID("B8FEBFE0-BAF8-454b-AECD-49FB91137B21"))) //PEP driver should use same UUID
    {

      // Number of fn IDs supported
      If(LEqual(Arg2, Zero))
      {
        Return(Buffer(One)
        {
          0x7
        })
      }

      // Pep presence
      If(LEqual(Arg2, One))
      {
        Store(0x1, PEPP)
        Return(0xf)
      }

      // Mitigation devices
      If(LEqual(Arg2, 0x2))
      {
        If(LEqual(Arg1, 0x0))
        {
          // Rev0
          Return(DEVS)
        }
        If(LEqual(Arg1, 0x1))
        {
          // Rev1
          Return(DEVX)
        }
      }

      // Crashdump device data
      If(LEqual(Arg2, 0x3))
      {
        Store("\\_SB.SDHA", Index(CDMP,0))
        Store(EM1A, Index(CDMP,1))
        Return(CDMP)
      }
    }
    // New UUID for built-in uPEP
    If(LEqual(Arg0,ToUUID("C4EB40A0-6CD2-11E2-BCFD-0800200C9A66")))
    {
      // Number of Functions (including this one)
      If(LEqual(Arg2, Zero))
      {
        Return(Buffer(One){0x7f})
      }
      // Device Constraints Enumeration
      If(LEqual(Arg2, 0x1))
      {
          // Update uPEP device list based on PEPC (Low Power S0 Constraint)
          // Bit[1:0] - SATA (0:None, 1:SATA Ports[all], 2:SATA Controller)
          //      [2] - En/Dis UART 0
          //      [3] -        UART 1
          //      [4] -        SDIO
          //      [5] -        I2C 0
          //      [6] -        I2C 1
          //      [7] -        XHCI
          //      [8] -        Azalia
          //      [9] -        ADSP
          //     [10] -        Not Used (previously PCIe SATA Controller)
          //     [11] -        Not Used (Previously PCIe SATA Port)
          //     [12] -        CPU
          //     [13] -        GFX
          //
		  Return(DEVY)
      }
      // Crashdump device data
      If(LEqual(Arg2, 0x2))
      {
        Store(EM1A, Local0)
        Add(Local0, 0x84, Local0)
        Store(Local0, Index(DerefOf(Index(DerefOf(Index(DerefOf(Index(DerefOf(Index(BCCD, Zero, )), One, )), Zero, )), Zero, )), 0x4, ))
        Return(BCCD)
      }
      // Screen off notification
      If(LEqual(Arg2, 0x3))
      {
        If(LEqual(S0IX, 1)) { //S0ID: >=1: CS 0: non-CS 
          // GUAM - Global User Absent Mode Notification Method
          //  \GUAM(One) // 0x01 - Power State Standby (CS Entry)    
        }  Else {
          // call method specific to non-CS platforms
        }
      }
      // Screen on notification
      If(LEqual(Arg2, 0x4))
      {
        If(LEqual(S0IX, 1)) { //S0ID: >=1: CS 0: non-CS 
          // GUAM - Global User Absent Mode Notification Method
          //\GUAM(ZERO) // 0x00 - Power State On (CS Exit)    
        }  
      }  Else {
         // call method specific to non-CS platforms
      }
      // resiliency phase entry (deep standby entry)
      If(LEqual(Arg2, 0x5))
      {
        If(LEqual(S0IX, 1)) { //S0ID: >=1: CS 0: non-CS 
        // call method specific to CS platforms when the system is in a
        // standby state with very limited SW activities   
        }
      }
      // resiliency phase exit (deep standby exit)
      If(LEqual(Arg2, 0x6))
      {
        If(LEqual(S0IX, 1)) { //S0ID: >=1: CS 0: non-CS 
          // call method specific to CS platforms when the system is in a
          // standby state with very limited SW activities   
        }
      }
      Return(One)
    }
  }
}
#endif

//
// eMMC 4.41
//
Device(SDHA)
{
  Name (_ADR, 0)
  Name (_HID, "80860F14")
  Name (_CID, "PNP0D40")
  Name (_DDN, "Intel(R) eMMC Controller - 80860F14")
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

  Name (RBF1, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {45}  // eMMC 4.41 IRQ
  })

  Method (_CRS, 0x0, NotSerialized)
  {
    // Update the Base address for BAR0 of eMMC 4.41
    CreateDwordField(^RBF1, ^BAR0._BAS, B0B1)
    CreateDwordField(^RBF1, ^BAR0._LEN, B0L1)
    Store(eM0A, B0B1)
    Store(eM0L, B0L1)
    Return (RBF1)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    //
    // PCIM>> 0:ACPI mode            1:PCI mode
    // SD1D>> 0:eMMC 4.41 enable     1:eMMC 4.41 disable
    //
    If (LAnd(LEqual(PCIM, 0), LEqual(SD1D, 0)))
    {
      Return (0xF)
    }
    Else
    {
      Return (0x0)
    }
  }


  Method (_PS3, 0, NotSerialized)
  {
//  OR(PSAT, 0x00000003, PSAT) //AMI_OVERRIDE - EIP137870 System can't run S4 with storage eMMC
    OR(PSAT, 0X00000000, PSAT)
    //
    // If not B1, still keep 2 ms w/a
    //
    If(LLess(SOCS, 0x03))
    {
      Sleep(2)
    }
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
    //
    // If not B1, still keep 2 ms w/a
    //
    If(LLess(SOCS, 0x03))
    {
      Sleep(2)
    }
  }

  OperationRegion (KEYS, SystemMemory, eM1A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,  32
  }

         Method (_DIS, 0x0, NotSerialized)
  {
    //Adding dummy disable methods for device EMM0
  }

  Device (EMMD)
  {
    Name (_ADR, 0x00000008) // Slot 0, Function 8
    Method (_RMV, 0, NotSerialized)
    {
      Return (0x0)
    }
  }
}


//
// eMMC 4.5
//
Device(SDHD)
{
  Name (_ADR, 0)
  Name (_HID, "80860F14")
  Name (_CID, "PNP0D40")
  Name (_DDN, "Intel(R) eMMC Controller - 80860F14")
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

  Name (RBF1, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {44}  // eMMC 4.5 IRQ
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBF1, ^BAR0._BAS, B0B1)
    CreateDwordField(^RBF1, ^BAR0._LEN, B0L1)
    Store(eM0A, B0B1)
    Store(eM0L, B0L1)
    Return (RBF1)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    //
    // PCIM>> 0:ACPI mode           1:PCI mode
    // HSID>> 0:eMMC 4.5 enable     1:eMMC 4.5 disable
    //
    If (LAnd(LEqual(PCIM, 0), LEqual(HSID, 0)))
    {
      Return (0xF)
    }
    Else
    {
      Return (0x0)
    }
  }


  Method (_PS3, 0, NotSerialized)
  {
//  OR(PSAT, 0x00000003, PSAT) //AMI_OVERRIDE - EIP137870 System can't run S4 with storage eMMC
    OR(PSAT, 0X00000000, PSAT)
    //
    // If not B1, still keep 2 ms w/a
    //
    If(LLess(SOCS, 0x03))
    {
      Sleep(2)
    }
  }
  Method (_PS0, 0, NotSerialized)
  {
    And(PSAT, 0xfffffffC, PSAT)
    OR(PSAT, 0X00000000, PSAT)
    //
    // If not B1, still keep 2 ms w/a
    //
    If(LLess(SOCS, 0x03))
    {
      Sleep(2)
    }
  }

  OperationRegion (KEYS, SystemMemory, eM1A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,  32
  }

         Method (_DIS, 0x0, NotSerialized)
  {
    //Adding dummy disable methods for device EMM0
  }

  Device (EM45)
  {
    Name (_ADR, 0x00000008) // Slot 0, Function 8
    Method (_RMV, 0, NotSerialized)
    {
      Return (0x0)
    }
  }
}


//
// SDIO
//
Device(SDHB)
{
  Name (_ADR, 0)
  Name (_HID, "INT33BB")
  Name (_CID, "PNP0D40")
  //Name (_CLS, Package (3) {0x08, 0x05, 0x01})
  Name (_DDN, "Intel(R) SDIO Controller - 80860F15")
  Name (_UID, 2)
  Name (_HRV, 2)

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif

  Name (PSTS, 0x0)

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {46}  // SDIO IRQ
  })

  Name (SBUF, ResourceTemplate ()
  {
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {46}  // SDIO IRQ
    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,"\\_SB.GPO2", 0x00, ResourceConsumer, ,)
    {
      // Pin list
      0x0015
    }
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    If (LEqual(OSEL, 1))
    {
      Return (SBUF)
    }

    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(SI0A, B0BA)
    Store(SI0L, B0LN)
    //CreateDwordField(^RBUF, BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, BAR1._LEN, B1LN)
    //Store(SI1A, B1BA)
    //Store(SI1L, B1LN)
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LLessEqual(STEP, 0x04))
    {
      //A stepping
      Store(SDMD, _HRV)
    }

    If (LOr(LEqual(SI0A, 0), LEqual(SD2D, 1)))
    {
      Return (0x0)
    }
    Return (0xF)
  }
  Method (_DIS, 0x0, NotSerialized)
  {
    //Adding dummy disable methods for device EMM0
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

    if(LEqual(\_SB.SDHB.PSTS,0x0))
    {
      if(LEqual (\_SB.GPO2.AVBL, 1))
      {
        //P8XH(0,0x58)   // Output 0x58 to Port 80h.
        Store( 0x01, \_SB.GPO2.WFD3 ) // WL_WIFI_REQ_ON = 1 put the device to normal state
        Store( 0x01, \_SB.SDHB.PSTS)  // indicates that the device powered ON
      }
    }


  }
  OperationRegion (KEYS, SystemMemory, SI1A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }


         Device (BRCM)
  {
    Name (_ADR, 0x01)                 //SlotNumber + Function

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

    Method (_RMV, 0, NotSerialized)
    {
      Return (0x0)
    }
    Name (_PRW, Package() {0, 0})
    Name (_S4W, 2)

    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {73}
        //GpioInt(Edge, ActiveHigh, ExclusiveAndWake, PullNone, 0,"\\_SB.GPO2") {15}         // COMBO_WLAN_IRQ     GPIO_SUS20 128+20
        //GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {20} // COMBO_WLAN _R_EN GPIO_SUS15 128+15
      })
      Return (RBUF)
    }

    Method (_PS3, 0, NotSerialized)
    {
      if(LEqual (\_SB.GPO2.AVBL, 1))
      {
        Store( 0x00, \_SB.GPO2.WFD3 ) // WL_WIFI_REQ_ON = 0 puts the device in reset state
        Store( 0x00, \_SB.SDHB.PSTS) //Indicates that the device is powered off
      }

    }
    Method (_PS0, 0, NotSerialized)
    {
      if(LEqual(\_SB.SDHB.PSTS,0x0))
      {
        if(LEqual (\_SB.GPO2.AVBL, 1))
        {
          Store( 0x01, \_SB.GPO2.WFD3 ) // WL_WIFI_REQ_ON = 1 put the device to normal state
          Store( 0x01, \_SB.SDHB.PSTS)     // indicates that the device powered ON
        }
      }
    }
  } // Device (BRCM)
  //
  // Secondary Broadcom WIFI function
  //
  Device(BRC2)
  {
    Name(_ADR, 0x2) // function 2
    Name(_STA, 0xf)
    //
    // The device is not removable. This must be a method.
    //
    Method(_RMV, 0x0, NotSerialized)
    {
      Return(0x0)
    }

    //
    // Describe a vendor-defined connection between this device and the
    // primary wifi device
    //

    Method(_CRS, 0, Serialized)
    {
      Name(NAM, Buffer() {"\\_SB.SDHB.BRCM"})
      Name(SPB, Buffer()
      {
        0x8E,       // SPB Descriptor
        0x18, 0x00, // Length including NAM above
        0x01,       // +0x00 SPB Descriptor Revision
        0x00,       // +0x01 Resource Source Index
        0xc0,       // +0x02 Bus type - vendor defined
        0x02,       // +0x03 Consumer + controller initiated
        0x00, 0x00, // +0x04 Type specific flags
        0x01,       // +0x06 Type specific revision
        0x00, 0x00  // +0x07 type specific data length
        // +0x09 - 0xf bytes for NULL-terminated NAM
        // Length = 0x18
      })

      Name(END, Buffer() {0x79, 0x00})
      Concatenate(SPB, NAM, Local0)
      Concatenate(Local0, END, Local1)
      Return(Local1)
    }
  }


  Device (BRC3)
  {
    Name (_ADR, 0x01)

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

    Name (_HID, "BCM4321")
    Name (_CID, "BCM43241")
    Name (GMOD, ResourceTemplate ()
    {
      GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", 0, ResourceConsumer, ,) {21}
    })
    
  OperationRegion(GPOP, SystemIo, \GPBS, 0x100)
  Field(GPOP, ByteAcc, NoLock, Preserve) {
      Offset(0x88),  // cfio_ioreg_SUS_GP_LVL_31_0_ - [GPIO_BASE_ADDRESS] + 88h
          ,  20,
      WFD3,  1
    }
 
               Method (_STA, 0x0, NotSerialized)
    {
      If (LEqual(OSEL, 1))
      {
        Return (0xF)
      }
      Return (0x0)
    }

    Method (_RMV, 0, NotSerialized)
    {
      Return (0x0)
    }
    Name (_PRW, Package() {0, 0})
    Name (_S4W, 2)
    Name (_S0W, 2)

    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {73}
        GpioIo (Exclusive, PullDefault,0,0,IoRestrictionOutputOnly,"\\_SB.GPO2",0,ResourceConsumer,,) {17}
      })
      Return (RBUF)
    }

    Method (_PS3, 0, NotSerialized)
    {
      if(LEqual (\_SB.GPO2.AVBL, 1))
      {
        Store( 0x00, \_SB.GPO2.WFD3) // WL_WIFI_REQ_ON = 0 puts the device in reset state
        Store( 0x00, \_SB.SDHB.PSTS) // Indicates that the device is powered off
      }

    }
    Method (_PS0, 0, NotSerialized)
    {
      if(LEqual(\_SB.SDHB.PSTS,0x0))
      {
        if(LEqual (\_SB.GPO2.AVBL, 1))
        {
          Store( 0x01, \_SB.GPO2.WFD3) // WL_WIFI_REQ_ON = 1 put the device to normal state
          Store( 0x01, \_SB.SDHB.PSTS) // Indicates that the device powered ON
        }
      }
    }
  } // Device (BRC3) for Android

}

//
// SD Card
//
Device(SDHC)
{
  Name (_ADR, 0)
  Name (_HID, "80860F16")
  Name (_CID, "PNP0D40")
  //Name (_CLS, Package (3) {0x08, 0x05, 0x01})
  Name (_DDN, "Intel(R) SD Card Controller - 80860F16")
  Name (_UID, 3)

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    

  Name (RBUF, ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR0)
    //Memory32Fixed (ReadWrite, 0x00000000, 0x00001000, BAR1)
    Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, , , ) {47}  // SD Card IRQ
    GpioInt (Edge, ActiveBoth, SharedAndWake, PullNone, 10000, "\\_SB.GPO0", 0x00, ResourceConsumer, , ) {38}//CD
    //GpioInt(Edge, ActiveBoth, Shared, PullNone,10000,"\\_SB.GPO0") {38}
    GpioIo (Shared, PullDefault, 0, 0, IoRestrictionInputOnly, "\\_SB.GPO0") {38} //DET
  })
  Method (_CRS, 0x0, NotSerialized)
  {
    CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
    CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
    Store(SD0A, B0BA)
    Store(SD0L, B0LN)
    //CreateDwordField(^RBUF, BAR1._BAS, B1BA)
    //CreateDwordField(^RBUF, BAR1._LEN, B1LN)
    //Store(SD1A, B1BA)
    //Store(SD1L, B1LN)
	
    Return (RBUF)
  }
  Method (_STA, 0x0, NotSerialized)
  {
    If (LOr(LEqual(SD0A, 0), LEqual(SD3D, 1)))
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
  OperationRegion (KEYS, SystemMemory, SD1A, 0x100)
  Field (KEYS, DWordAcc, NoLock, WriteAsZeros)
  {
    Offset (0x84),
           PSAT,   32
  }
}

