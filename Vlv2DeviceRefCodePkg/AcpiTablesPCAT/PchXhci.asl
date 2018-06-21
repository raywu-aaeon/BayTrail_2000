/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Haswell             *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  2013 - 2014 Intel Corporation. All rights reserved   *;
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

//scope is \_SB.PCI0.XHC
//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT
scope (\_SB.PCI0.XHC1) 
{
#else
Device(XHC1)
{
  Name(_ADR, 0x00140000)                     //Device 20, Function 0
#endif
//AMI_OVERRIDE <<

  //When it is in Host mode, USH core is connected to USB3 microAB(USB3 P1 and USB2 P0)
  Name (_DDN, "Baytrail XHCI controller (CCG core/Host only)" )

#ifdef WIN8_SUPPORT     
    Method(_DEP){
    	if (LAnd(LGreaterEqual(OSYS, 2013), LEqual(S0IX, 1))) {
			Return(Package() {\_SB.PEPD})
		}Else{
			Return(Package(){})
		}
	}
#endif    
  Name (_STR, Unicode ("Baytrail XHCI controller (CCG core/Host only)"))
  Name(_PRW, Package() {0xD,4})

  Method(_PSW,1)
  {
    If (LAnd (PMES, PMEE)) {
       Store (0, PMEE)
       Store (1, PMES)
    }
  }

  OperationRegion (PMEB, PCI_Config, 0x74, 0x04)  //Cspec usbx_top.doc.xml 1.2.3.27     Power Management Control/Status
  Field (PMEB, WordAcc, NoLock, Preserve)
  {
    ,   8,
    PMEE,   1,   //bit8 PME_En
    ,   6,
    PMES,   1    //bit15 PME_Status
  }

  OperationRegion(NV0, SystemIO, 0x72, 0x2)
  Field(NV0, ByteAcc, NoLock, Preserve)
  {
    IND0, 0x00000008, 
    DAT0, 0x00000008, 
  }

  // Use ACPI Defined IndexField so consecutive Index/Data I/Os are 
  // assured to be uninterrupted.

  IndexField(IND0, DAT0, ByteAcc, NoLock, Preserve)
  {
    Offset(ASL_XHCI_MODE_CMOS), //AMI_OVERRIDE - EIP154014 Use token to decide CMOS offset. 
    XUSB, 1, 
  }

  Method(_STA, 0)
  {
    If(LNotEqual(XHCI, 0))      //NVS variable controls present of XHCI controller
    {
      Return (0xF)
    } Else
    {
      Return (0x0)
    }
  }

  OperationRegion(XPRT,PCI_Config,0xD0,0x10)
  Field(XPRT,DWordAcc,NoLock,Preserve)       //usbx_top.doc.xml
  {
    PR2,  32,                              //bit[8:0] USB2HCSEL
    PR2M, 32,                              //bit[8:0] USB2HCSELM
    PR3,  32,                              //bit[3:0] USB3SSEN
    PR3M, 32                               //bit[3:0] USB3SSENM
  }

#ifdef WIN7_SUPPORT
    Name(XRST, Zero)
  
    //
    //
    // Check for XHCI switch UUID
    //
    // Arguments:
    //  Arg0 (Buffer) : UUID
    //
    // Returns:
    //  1: It's valid UUID
    //  0: Invalid UUID
    //
    Method(CUID,1,Serialized) {
      If(LEqual(Arg0,ToUUID("7c9512a9-1705-4cb4-af7d-506a2423ab71"))) {
        Return(1)
      }
      Return(0)
    }

    //
    // _OSC for xHCI
    // This method enables XHCI controller if available.
    //
    // Arguments:
    //  Arg0 (Integer): Revision ID - should be set to 1
    //  Arg1 (Integer): Count
    //  Arg2 (Buffer) : Capabilities Buffer
    //                  DWORD#1 (Status/Error):
    //                  Bit 0 - Query Support Flag
    //                  Bit 1 - Always clear(0)
    //                  Bit 2 - Always clear(0)
    //                  Bit 3 - Always clear(0)
    //                  All others - reserved (return 0)
    //
    //                  DWORD#3 (Controlled):
    //                  Bit 0 - If set OS request routing back to EHCI
    //                  All others - reserved (return 0)
    // Returns:
    //  Capabilities Buffer: 
    //                  DWORD#1 (Status):
    //                  Bit 0 - Reserved (not used) 
    //                  Bit 1 - _OSC failure. Platform Firmware was unable to process the request or query.
    //                          Capabilities bits may have been masked. 
    //                  Bit 2 - Unrecognized UUID. This bit is set to indicate that the platform firmware does not
    //                          recognize the UUID passed in _OSC Arg0.
    //                          Capabilities bits are preserved. 
    //                  Bit 3 - Unrecognized Revision. This bit is set to indicate that the platform firmware does not
    //                          recognize the Revision ID passed in via Arg1.
    //                          Capabilities bits beyond those comprehended by the firmware will be masked. 
    //                  Bit 4 - Capabilities Masked. This bit is set to indicate 
    //                          that capabilities bits set by driver software
    //                          have been cleared by platform firmware. 
    //                  All others - reserved (return 0)
    //
    Method(POSC,3,Serialized) {
      //
      // Create DWord field from the Capabilities Buffer
      //
      CreateDWordField(Arg2,0,CDW1)
      CreateDWordField(Arg2,8,CDW3)
      //
      // Check revision
      //
      If(LNotEqual(Arg1,One)) {
        //
        // Set unknown revision bit
        //
        Or(CDW1,0x8,CDW1)
      }

      //
      // Set failure if xHCI is disabled by BIOS
      //
      If (LEqual(XHCI, 0)) {
        Or(CDW1,0x2,CDW1)
      }

      //
      // Query flag clear
      //
      If(LNot(And(CDW1,0x1))) {
        If (And(CDW3,0x1)) {
          //
          // Perform switch back to EHCI
          //
          ESEL()
        }
        Else {
          //
          // Perform switch to xHCI
          //
          XSEL()
          
          If (LOr(LEqual(XHCI,2), LEqual(XHCI,3))) {
           
            Store(0x03, \_SB.PCI0.EHC1.PSTA)
            Sleep(100)
            Store(0x00, \_SB.PCI0.EHC1.UMBA)
            Sleep(100)
            Store(0x00, \_SB.PCI0.EHC1.PCMD)
            Sleep(100)
            OperationRegion(PMCS,SystemMemory,0xFED03000,0x40)
            Field(PMCS,DWordAcc,NoLock,Preserve)
            {
              Offset(0x34),
                  , 18,
              DUSB,  1
            }
            Store(0x01,DUSB)
            Sleep(100)
          }
        }
      }
      Return(Arg2)
    }

    Method(XSEL, 0, Serialized)
    {
      OperationRegion(XBAS,SystemMemory,0xFFFF0000,0x500)
      Field(XBAS,DWordAcc,NoLock,Preserve)
      {
        Offset(0x480),
        POR1, 32,
        Offset(0x490),
        POR2, 32,
        Offset(0x4A0),
        POR3, 32,
        Offset(0x4B0),
        POR4, 32,
        Offset(0x4C0),
        POR5, 32,
        Offset(0x4D0),
        POR6, 32,
        Offset(0x4E0),
        PORX, 32                                      
      }
      
      OperationRegion(EBAS,SystemMemory,0xFFFF5000,0x30)
      Field(EBAS,DWordAcc,NoLock,Preserve)
      {
        Offset(0x20),
        EBST, 1 
      }

      //
      // xHCI in auto or smart auto mode
      //
      If (LOr(LEqual(XHCI,2), LEqual(XHCI,3))) {
        //
        // Set B0:D31:F0 ACh[16] to indicate begin of Driver phase of USB port routing
        //
        Store(1, XUSB)
        Store(1, XRST) // Backup XUSB, cause it might lost in iRST G3 or DeepSx
        
        //
        // Enable selected SS ports, route corresponding HS ports to xHCI
        //

        //
        // 1.Stop USB 2.0 bus
        // 2.Route HS port to xHCI
        // 3.Reset USB 2.0 port
        // 4.Reset USB 3.0 port
        //
        Store(0x00, EBST)
        
        Store(0x0000003F,PR2M)
        Store(0x00000001,PR3M)
        Store(0x00000001,PR3)
        Store(0x0000003F,PR2)
        If (LNotEqual(WAKS, 3))
        {        
          Or(0x00002100, POR1, POR1)
          Or(0x00002100, POR2, POR2)
          Or(0x00002100, POR3, POR3)
          Or(0x00002100, POR4, POR4)
          Or(0x00002100, POR5, POR5)
          Or(0x00002100, POR6, POR6)
          Stall(100)

          Store(PORX,Local0)
          Or(0x80000000,Local0,PORX)
          Stall(100)
        }        
      }
    }

    Method(ESEL, 0, Serialized)
    {
      //
      // xHCI in auto or smart auto mode 
      //
      If (LOr(LEqual(XHCI,2), LEqual(XHCI,3))) {
        //
        // Disable all SS ports, route all HS ports to EHCI
        //
        And(PR3, 0xFFFFFFFE, PR3)
        And(PR2, 0xFFFFFFC0, PR2)

        //
        // Mark as not routed.
        //
        Store(0, XUSB)
        Store(0, XRST)
      }
    }

    Method(XWAK, 0, Serialized)
    {
      //
      // Ports were routed to xHCI before sleep
      //
      If (LOr(LEqual(XUSB,1), LEqual(XRST,1))) {
//      If (LEqual(XRST,1)) {

        //
        // Restore back to xHCI
        //
        XSEL()
      }
    }
#endif 
  Device(RHUB)
  {
    Name(_ADR, Zero)         //address 0 is reserved for root hub

    //
    // High Speed Ports
    // It would be USB2.0 port if SS termination is disabled
    // USB3 type-A port is in the docking card(RVP)


    //
    // Super Speed Ports - must match _UPC declarations of the coresponding Full Speed Ports.
    //   Paired with Port 1
    Device(SSP1)
    {
      Name(_ADR, 0x07)

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                                      // Port is connectable if non-zero
          0x06,                                      // USB3 uAB connector
          0x00,
          0x00
        })
        Return(UPCP)
      }

      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()       //pls check ACPI 5.0 section 6.1.8
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'011 visiable/docking/no lid bit[69:67]=b'001 bottom panel bit[71:70]=b'01 Center  bit[73:72]=b'01 Center
            //           bit[77:74]=6 Horizontal Trapezoid bit[78]=0 bit[86:79]=0 bit[94:87]='1 no group info' bit[95]=0 not a bay
            0x4B, 0x19, 0x80, 0x00,
            //127:96 -bit[96]=1 Ejectable bit[97]=1 OSPM Ejection required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x03, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
        Return (PLDP)
      }
    }
    //
    // High Speed Ports
    // pair port with port 7 (SS)
    //    The UPC declarations for LS/FS/HS and SS ports that are paired to form a USB3.0 compatible connector.
    //    A "pair" is defined by two ports that declare _PLDs with identical Panel, Vertical Position, Horizontal Postion, Shape, Group Orientation
    //    and Group Token
    Device(HS01)
    {
      Name(_ADR, 0x01)

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package() { 0xFF,0x06,0x00,0x00 })
        Return(UPCP)
      }

      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()       //pls check ACPI 5.0 section 6.1.8
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'011 visiable/docking/no lid bit[69:67]=b'001 bottom panel bit[71:70]=b'01 Center  bit[73:72]=b'01 Center
            //           bit[77:74]=6 Horizontal Trapezoid bit[78]=0 bit[86:79]=0 bit[94:87]='1 no group info' bit[95]=0 not a bay
            0x4B, 0x19, 0x80, 0x00,
            //127:96 -bit[96]=1 Ejectable bit[97]=1 OSPM Ejection required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x03, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
        Return (PLDP)
      }
    }//end of HS01

    // USB2 Type-A/USB2 only   (J6A1 connector on RVP)
    // EHCI debug capable
    Device(HS02)
    {
      Name(_ADR, 0x02)                                   // 0 is for root hub so physical port index starts from 1 (it is port1 in schematic)

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                     // connectable
          0xFF,                     //
          0x00,
          0x00
        })

        Return(UPCP)                                     // it connects to DEBUG card on FFRD.  it is USB2 port on RVP.
      }

      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'000 not visiable/no docking/no lid bit[69:67]=b'000 top bit[71:70]=b'01 Center  bit[73:72]=b'00 Left
            //           bit[77:74]=2 Square bit[78]=0 bit[86:79]=0 bit[94:87]='2 no group info' bit[95]=0 not a bay
            //0x40, 0x08, 0x00, 0x01,   // AMI_OVERRIDE - EIP245223 USB port2 HighSpeed safety removable icon
            0x41, 0x08, 0x00, 0x01,
            //127:96 -bit[96]=0 not Ejectable bit[97]=0 no OSPM Ejection required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x00, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })

        Return (PLDP)
      }
    }//end of HS02
    // high speed port 3  (USB2_P2 in RVP schematic) FPC connector J6B1
    Device(HS03)
    {
      Name(_ADR, 0x03)                                   // to WWAN module on both RVP and FFRD

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                     //  connectable
          0xFF,
          0x00,
          0x00
        })

        Return(UPCP)
      }

#ifdef ASL_OEM_USB_HARDWARE_RMV  //AMI_OVERRIDE - EIP145181 According to OEM USB hardware desgin to use _RMV.
      Method(_RMV, 0)                                    // for XHCICV debug purpose
      {
        Return(0x0)
      }
#endif //AMI_OVERRIDE - EIP145181 According to OEM USB hardware desgin to use _RMV.

      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'000 not Visible/no docking/no lid bit[69:67]=6 (b'110) unknown(Vertical Position and  Horizontal Position will be ignored)
            //           bit[71:70]=b'00 Vertical Position ignore bit[73:72]=b'00 Horizontal Position ignore
            //           bit[77:74]=2 Square bit[78]=0 bit[86:79]=0 bit[94:87]='3 no group info' bit[95]=0 not a bay
            0x30, 0x08, 0x80, 0x01,
            //127:96 -bit[96]=0 not Ejectable bit[97]=0 OSPM Ejection not required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x00, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
        Return (PLDP)
      }
    }

    Device(HS04)
    {
      Name(_ADR, 0x04)                                   // FPC port on RVP. Not connected on FFRD.

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                     //connectable
          0xFF,                     //Proprietary connector (FPC connector)
          0x00,
          0x00
        })

        Return(UPCP)
      }
      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'000 not Visible/no docking/no lid bit[69:67]=6 (b'110) unknown(Vertical Position and  Horizontal Position will be ignored)
            //           bit[71:70]=b'00 Vertical Position ignore bit[73:72]=b'00 Horizontal Position ignore
            //           bit[77:74]=2 Square bit[78]=0 bit[86:79]=0 bit[94:87]='4 no group info' bit[95]=0 not a bay
            0x30, 0x08, 0x00, 0x02,
            //127:96 -bit[96]=0 not Ejectable bit[97]=0 OSPM Ejection not required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x00, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })

        Return (PLDP)
      }
    }


    Device(HSC1)                                           // USB2 HSIC 01
    {
      Name(_ADR, 0x05)

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                     //connectable
          0xFF,                     //Proprietary connector (FPC connector)
          0x00,
          0x00
        })

        Return(UPCP)
      }
      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'000 not Visible/no docking/no lid bit[69:67]=6 (b'110) unknown(Vertical Position and  Horizontal Position will be ignored)
            //           bit[71:70]=b'00 Vertical Position ignore bit[73:72]=b'00 Horizontal Position ignore
            //           bit[77:74]=2 Square bit[78]=0 bit[86:79]=0 bit[94:87]='5 no group info' bit[95]=0 not a bay
            0x30, 0x08, 0x80, 0x02,
            //127:96 -bit[96]=0 not Ejectable bit[97]=0 OSPM Ejection not required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x00, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
        Return (PLDP)
      }
    }

    Device(HSC2)                                           // USB2 HSIC 02
    {
      Name(_ADR, 0x06)

      Method(_UPC,0,Serialized)
      {
        Name(UPCP, Package()
        {
          0xFF,                     //connectable
          0xFF,                     //Proprietary connector (FPC connector)
          0x00,
          0x00
        })

        Return(UPCP)
      }
      Method(_PLD,0,Serialized)
      {
        Name(PLDP, Package()
        {
          Buffer(0x14)
          {
            //31:0   - Bit[6:0]=2 revision is 0x2, Bit[7]=1 Ignore Color Bit[31:8]=0 RGB color is ignored
            0x82, 0x00, 0x00, 0x00,
            //63:32 - Bit[47:32]=0 width: 0x0000  Bit[63:48]=0 Height:0x0000
            0x00, 0x00, 0x00, 0x00,
            //95:64 - bit[66:64]=b'000 not Visible/no docking/no lid bit[69:67]=6 (b'110) unknown(Vertical Position and  Horizontal Position will be ignored)
            //           bit[71:70]=b'00 Vertical Position ignore bit[73:72]=b'00 Horizontal Position ignore
            //           bit[77:74]=2 Square bit[78]=0 bit[86:79]=0 bit[94:87]='6 no group info' bit[95]=0 not a bay
            0x30, 0x08, 0x00, 0x03,
            //127:96 -bit[96]=0 not Ejectable bit[97]=0 OSPM Ejection not required Bit[105:98]=0 no Cabinet Number
            //            bit[113:106]=0 no Card cage Number bit[114]=0 no reference shape Bit[118:115]=0 no rotation Bit[123:119]=0 no order
            0x00, 0x00, 0x00, 0x00,
            //159:128  Vert. and Horiz. Offsets not supplied
            0xFF, 0xFF, 0xFF, 0xFF
          }
        })
        Return (PLDP)
      }
    }
  }  //end of root hub

} // end of XHC1

//scope is \_SB.PCI0.OTG1
//only OTG device mode is supported
Device(OTG1)
{
  Name(_ADR, 0x00160000)                     //B0:D22:F0   0x16=22

  //When it is in Device mode, Synopsys core takes care of USB3 microAB (USB3 P0 and ULPI).
  Name (_DDN, "Baytrail XHCI controller (Synopsys core/OTG)" )
  Name (_STR, Unicode ("Baytrail XHCI controller (Synopsys core/OTG)"))

  //remove _PSW

  OperationRegion (PMEB, PCI_Config, 0x84, 0x04)  //CSPEC otg3.doc.xml 1.2.1.12       PMECTRLSTATUS
  Field (PMEB, WordAcc, NoLock, Preserve)
  {
    ,   8,
    PMEE,   1,    //bit8 PMEENABLE
    ,   6,
    PMES,   1     //bit15 PMESTATUS
  }

  OperationRegion (GENR, PCI_Config, 0xA0, 0x10)  //CSPEC otg3.doc.xml 1.2.2.1  map_otg_IOSF2OCP_CONFIGREG_pci
  Field (GENR, WordAcc, NoLock, Preserve)
  {
    ,   18,
    CPME,   1,    //bit18 core_pme_en
    U2EN,    1,    //bit19 u2_pme_en
    U3EN,   1     //bit20 u3_pme_en
  }

  Method (_PS3, 0, NotSerialized)
  {
    Store (One, CPME)
    Store (One, U2EN)
    Store (One, U3EN)
  }
  Method (_PS0, 0, NotSerialized)
  {
    Store (Zero, CPME)
    Store (Zero, U2EN)
    Store (Zero, U3EN)
  }

  Method (_DSW, 3, NotSerialized)   // _DSW: Device Sleep Wake
  {
  }

  Method (_RMV, 0, NotSerialized)   // _RMV: Removal Status
  {
    Return (Zero)
  }

  Method (_PR3, 0, NotSerialized)   // _PR3: Power Resources for D3hot
  {
    Return (Package (0x01)
    {
      USBC
    })
  }

  Method (_STA, 0x0, NotSerialized)
  {
    If(LNotEqual(OTGM, 0))      //NVS variable controls present of  OTG controller
    {
      Return (0xF)
    } Else
    {
      Return (0x0)
    }
  }

} // end of OTG1
