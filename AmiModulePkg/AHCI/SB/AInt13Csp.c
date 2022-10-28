//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2017, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file AInt13Csp.c
    AHCI INT13 Support Chip set Configuration File

**/
//---------------------------------------------------------------------------

#include "AmiDxeLib.h"
#include "Protocol/PciIo.h"

//---------------------------------------------------------------------------

#define LBAR_REGISTER		    0x20
#define LBAR_ADDRESS_MASK	    0xFFFFFFE0
#define INDEX_OFFSET_FROM_LBAR  0x10
#define DATA_OFFSET_FROM_LBAR   0x14



/**
    This is chip set porting routine that returns index/data ports
    to access memory-mapped registers.

    @param    PciIo

    @retval    EFI_SUCCESS Access information is collected
    @retval    EFI_ACCESS_DENIED No Access information available

**/

EFI_STATUS
GetAccessInfo (
    IN EFI_PCI_IO_PROTOCOL *PciIo,
    OUT UINT16  *AccessIndexPort,
    OUT UINT16  *AccessDataPort
)
{
    EFI_STATUS Status;
    UINT32 lbar;

    Status = PciIo->Pci.Read(PciIo, EfiPciIoWidthUint32, LBAR_REGISTER, 1, &lbar);
    ASSERT_EFI_ERROR(Status);

    lbar &= LBAR_ADDRESS_MASK;  // Legacy Bus Master Base Address

    *AccessIndexPort = (UINT16)lbar + INDEX_OFFSET_FROM_LBAR;
    *AccessDataPort = (UINT16)lbar + DATA_OFFSET_FROM_LBAR;
    
    // If index/Data port is invalid return unsupported, also by default 
    // index/Data port is 0xffff in AhciAcc.csm16
    if ( (*AccessIndexPort == 0) || (*AccessDataPort == 0 ) ) {
        return EFI_UNSUPPORTED;
    }

    // --------------
	// Return index/Data port as zero with EFI_SUCCESS, in case Index/data port access is done in any other method ( eg. PCI Index/Data port ).
	// This needs porting to be done in AhciAcc.csm16 in place of normal Index/Data port access.
    // --------------

    return EFI_SUCCESS;

}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2017, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
