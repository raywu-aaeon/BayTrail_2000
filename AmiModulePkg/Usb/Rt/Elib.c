//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/elib.c 19    8/29/12 8:16a Ryanchou $
//
// $Revision: 19 $
//
// $Date: 8/29/12 8:16a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           Elib.c
//
//  Description:    AMI USB MEM/IO/PCI access routines
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"

extern  USB_GLOBAL_DATA         *gUsbData;
//extern  EFI_SMM_SYSTEM_TABLE    *gSmst;

UINT8   ByteReadIO(UINT16);
UINT16  WritePCIConfig(UINT16, UINT8);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FixedDelay
//
// Description: This routine delays for specified number of micro seconds
//
// Input:   wCount      Amount of delay (count in 1 microsec)
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID 
FixedDelay(
    UINTN           Usec                           
 )
{
#if USB_RUNTIME_DRIVER_IN_SMM
    UINTN   Counter, i;
    UINT32  Data32, PrevData;

    Counter = Usec * 3;
    Counter += Usec / 2;
    Counter += (Usec * 2) / 25;

    //
    // Call WaitForTick for Counter + 1 ticks to try to guarantee Counter tick
    // periods, thus attempting to ensure Microseconds of stall time.
    //
    if (Counter != 0) {

        PrevData = IoRead32(PM_BASE_ADDRESS + 8);
        for (i=0; i < Counter; ) {
            Data32 = IoRead32(PM_BASE_ADDRESS + 8);    
            if (Data32 < PrevData) {        // Reset if there is a overlap
                PrevData=Data32;
                continue;
            }
            i += (Data32 - PrevData);        
            PrevData = Data32;
        }
    }
#else
	pBS->Stall(Usec);
#endif
    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordReadMem
//
// Description: This routine reads a DWORD from the specified Memory Address
//
// Input:   dBaseAddr   - Memory address to read
//          bOffset     - Offset of dBaseAddr
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
DwordReadMem(UINT32 dBaseAddr, UINT16 wOffset)
{
    return *(volatile UINT32*)(UINTN)(dBaseAddr+wOffset);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordWriteMem
//
// Description: This routine writes a DWORD to a specified Memory Address
//
// Input:   dBaseAddr   - Memory address to write
//          bOffset     - Offset of dBaseAddr
//          dValue      - Data to write
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordWriteMem(UINT32 dBaseAddr, UINT16 wOffset, UINT32 dValue)
{
    *(volatile UINT32*)(UINTN)(dBaseAddr+wOffset) = dValue;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordResetMem
//
// Description: This routine resets the specified bits at specified memory address
//
// Input:   dBaseAddr   - Memory address to read
//          bOffset     - Offset of dBaseAddr
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordResetMem(UINT32 dBaseAddr, UINT16 wOffset, UINT32 dValue)
{
    UINT32 data = DwordReadMem(dBaseAddr, wOffset);
    data &= ~dValue;
    DwordWriteMem(dBaseAddr, wOffset, data);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordSetMem
//
// Description: This routine sets the specified bits at specified memory address
//
// Input:   dBaseAddr   - Memory address to read
//          bOffset     - Offset of dBaseAddr
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordSetMem(UINT32 dBaseAddr, UINT16 wOffset, UINT32 dValue)
{
    UINT32 data = DwordReadMem(dBaseAddr, wOffset);
    data |= dValue;
    DwordWriteMem(dBaseAddr, wOffset, data);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ByteReadIO
//
// Description: This routine reads a Byte from the specified IO address
//
// Input:   wIOAddr     I/O address to read
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
ByteReadIO(UINT16 wIOAddr)
{
//    UINT8 value;
//    gSmst->SmmIo.Io.Read(&gSmst->SmmIo, SMM_IO_UINT8, (UINT64)wIOAddr, 1, &value);
//    return value;
    return IoRead8(wIOAddr);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ByteWriteIO
//
// Description: This routine writes a byte to the specified IO address
//
// Input:   wIOAddr     I/O address to write
//          bValue      Byte value to write
//
// Output:  None
//
// Modified:    Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ByteWriteIO (UINT16 wIOAddr, UINT8 bValue)
{
//    gSmst->SmmIo.Io.Write(&gSmst->SmmIo, SMM_IO_UINT8, (UINT64)wIOAddr, 1, &bValue);
    IoWrite8(wIOAddr, bValue);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WordReadIO
//
// Description: This routine reads a Word from the specified IO address
//
// Input:   wIOAddr     I/O address to read
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
WordReadIO(UINT16 wIOAddr)
{
//    UINT16 value;
//    gSmst->SmmIo.Io.Read(&gSmst->SmmIo, SMM_IO_UINT16, (UINT64)wIOAddr, 1, &value);
//    return  value;
    return IoRead16(wIOAddr);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WordWriteIO
//
// Description: This routine writes a word to the specified IO address
//
// Input:   wIOAddr     I/O address to write
//          wValue      Word value to write
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
WordWriteIO (UINT16 wIOAddr, UINT16 wValue)
{
//    gSmst->SmmIo.Io.Write(&gSmst->SmmIo, SMM_IO_UINT16, (UINT64)wIOAddr, 1, &wValue);
    IoWrite16(wIOAddr, wValue);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordReadIO
//
// Description: This routine reads a dword from the specified IO address
//
// Input:   wIOAddr     I/O address to read
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
DwordReadIO(UINT16 wIOAddr)
{
//    UINT32  value;
//    gSmst->SmmIo.Io.Read(&gSmst->SmmIo, SMM_IO_UINT32, (UINT64)wIOAddr, 1, &value);
//    return  value;
    return IoRead32(wIOAddr);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordWriteIO
//
// Description: This routine writes a double word to the specified IO address
//
// Input:   wIOAddr     I/O address to write
//      dValue      Double word value to write
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordWriteIO(UINT16 wIOAddr, UINT32 dValue)
{
//    gSmst->SmmIo.Io.Write(&gSmst->SmmIo, SMM_IO_UINT32, (UINT64)wIOAddr, 1, &dValue);
    IoWrite32(wIOAddr, dValue);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadPCIConfig
//
// Description: This routine reads from the PCI configuration space register
//              the value can be typecasted to 8bits - 32bits
//
// Input:   BusDevFunc - Bus, device & function number of the PCI device
//          Register   - Register offset to read
//
// Output:  Value read
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
ReadPCIConfig(UINT16 BusDevFunc, UINT8 Register)
{
    UINT32 data;
    DwordWriteIO(0xCF8, (UINT32)(0x80000000 | (BusDevFunc<<8) | (Register & 0xFC)));
    data = DwordReadIO(0xCFC);
    return (data >> ((Register & 3) << 3)); // Adjust uneven register

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ByteWritePCIConfig
//
// Description: This routine writes a byte value to the PCI configuration
//      register space
//
// Input:   BusDevFunc - Bus, device & function number of the PCI device
//      Register   - Register offset to read
//      Value      - Value to write
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ByteWritePCIConfig(UINT16 BusDevFunc, UINT8 Register, UINT8 Value)
{
    UINT16 wIOAddr;
    wIOAddr = WritePCIConfig(BusDevFunc, Register);
    ByteWriteIO (wIOAddr, Value);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WordWritePCIConfig
//
// Description: This routine writes a byte value to the PCI configuration
//      register space
//
// Input:   BusDevFunc - Bus, device & function number of the PCI device
//      Register   - Register offset to read
//      Value      - Value to write
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
WordWritePCIConfig(UINT16 BusDevFunc, UINT8 Register, UINT16 Value)
{
    UINT16 wIOAddr;
    wIOAddr = WritePCIConfig(BusDevFunc, Register);
    WordWriteIO (wIOAddr, Value);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DwordWritePCIConfig
//
// Description: This routine writes a Dword value to the PCI configuration
//      register space
//
// Input:   BusDevFunc - Bus, device & function number of the PCI device
//      Register   - Register offset to read
//      Value      - Value to write
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
DwordWritePCIConfig(UINT16 BusDevFunc, UINT8 Register, UINT32 Value)
{
    UINT16 wIOAddr;
    wIOAddr = WritePCIConfig(BusDevFunc, Register);
    DwordWriteIO (wIOAddr, Value);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   WritePCIConfig
//
// Description: This function opens PCI configuration for a given register
//
// Input:   wBDF  - Bus, device and function number
//          bReg  - Register number to read
//
// Output:  IO register to write the value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
WritePCIConfig(UINT16 wBDF, UINT8 bReg)
{
    DwordWriteIO(0xCF8, (UINT32)(0x80000000 | (wBDF<<8) | (bReg & 0xFC)));
    return (UINT16)(0xCFC+(bReg & 3));
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SpeakerBeep
//
// Description: This routine produces a sound on the internal PC speaker
//
// Input:   bFreq -     Sound frequency
//      wDuration - Sound duration in 15 microsecond units
//      fpHCStruc - Pointer to HCStruc
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
SpeakerBeep (UINT8 bFreq, UINT16 wDuration, HC_STRUC*   fpHCStruc)
{
#if USB_BEEP_ENABLE
    UINT8   bValue;
    if(gUsbData->dUSBStateFlag & USB_FLAG_ENABLE_BEEP_MESSAGE) {
        ByteWriteIO((UINT8)0x43, (UINT8)0xB6);
        ByteWriteIO((UINT8)0x42, (UINT8)bFreq);
        ByteWriteIO((UINT8)0x42, (UINT8)bFreq);
        bValue = ByteReadIO((UINT8)0x61);
        ByteWriteIO((UINT8)0x61, (UINT8)(bValue | 03));
        FixedDelay((UINTN)wDuration * 15);
        ByteWriteIO((UINT8)0x61, (UINT8)(bValue));
    }
#endif
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
