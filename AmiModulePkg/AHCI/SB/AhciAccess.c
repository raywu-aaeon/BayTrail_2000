//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file AhciAccess.c
    Provides Index Data Port Access to AHCI Controller

**/
//**********************************************************************
//#include <AmiDxeLib.h>

#define LBAR_REGISTER           0x20
#define LBAR_ADDRESS_MASK       0xFFFFFFE0
#define INDEX_OFFSET_FROM_LBAR  0x10
#define DATA_OFFSET_FROM_LBAR   0x14

#include "AmiDxeLib.h"
#include "Protocol/PciIo.h"

UINT16 IndexPort, DataPort;

/**
    This is chip set porting routine that returns index/data ports
    to access memory-mapped registers.

    @param PciIo

    @retval EFI_SUCCESS Access information is collected
    @retval EFI_ACCESS_DENIED No Access information available

**/

EFI_STATUS
InitilizeIndexDataPortAddress (
    IN EFI_PCI_IO_PROTOCOL *PciIo
)
{
    EFI_STATUS Status;
    UINT32 lbar;

    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, LBAR_REGISTER, 1, &lbar);
    ASSERT_EFI_ERROR(Status);

    lbar &= LBAR_ADDRESS_MASK;  // Legacy Bus Master Base Address

    IndexPort = (UINT16)lbar + INDEX_OFFSET_FROM_LBAR;
    DataPort = (UINT16)lbar + DATA_OFFSET_FROM_LBAR;

    return EFI_SUCCESS;
}

/**
    Read the Dword Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to read           

    @retval Value Read

**/
UINT32
ReadDataDword (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    return IoRead32(DataPort);
}
/**
    WriteRead the Dword Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to Write
    @param    Data        - Data to be written        

    @retval    Nothing

**/
VOID
WriteDataDword (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index, 
    IN  UINTN   Data
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    IoWrite32(DataPort, (UINT32)Data);
}

/**
    Read the Word Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to read           

    @retval Value Read

**/
UINT16
ReadDataWord (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    return (UINT16)IoRead32(DataPort);
}
/**
    WriteRead the word Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to Write
    @param    Data        - Data to be written        

    @retval    Nothing

**/
VOID
WriteDataWord (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index, 
    IN  UINTN   Data
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    IoWrite32(DataPort, (UINT16)Data);
}
/**
    Read the Byte Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to read           

    @retval    Value Read

**/
UINT8
ReadDataByte (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    return (UINT8)IoRead32(DataPort);
}
/**
    WriteRead the Dword Data using Index/Data access method

    @param    BaseAddress - BaseAddress of AHCI Controller
    @param    Index       - Index address to Write
    @param    Data        - Data to be written        

    @retval Nothing

**/
VOID
WriteDataByte (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index,
    IN  UINTN   Data
)
{
    IoWrite32(IndexPort, (UINT32)Index);
    IoWrite8(DataPort, (UINT8)Data);
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************