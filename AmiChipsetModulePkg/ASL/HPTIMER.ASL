//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/BIN/Chipset/Intel/SouthBridge/CougarPoint_A/Chipset/ASL/Hptimer.asl 1     2/24/10 11:04p Tonywu $
//
// $Revision: 1 $
//
// $Date: 2/24/10 11:04p $
//**********************************************************************;
//**********************************************************************;
//-------------------------
// High Performance Event Timer (HPET)
// Intel iICH example
//-------------------------
Scope(\_SB) {
  Device(HPET)	// High Performance Event Timer
  {
    Name (_HID, EisaId ("PNP0103"))
    Name (_UID, 0x00)
    Method (_STA, 0, NotSerialized)
    {
      Return (0x0F)
    }

    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
         Memory32Fixed (ReadWrite,
                        0xFED00000,         // Address Base
                        0x00000400,         // Address Length
                        )
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
        {
          0x00000008,   //0xB HPET-2
        }
     })
     Return (RBUF)
   }
 }
//HPET
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
