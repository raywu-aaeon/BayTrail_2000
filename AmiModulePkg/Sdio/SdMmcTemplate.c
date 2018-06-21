//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//<AMI_FHDR_START>
//****************************************************************************
// Name:        SdMmcTemplate.c
//
// Description: This file is template for chipset specific programming
//
//****************************************************************************
//<AMI_FHDR_END>

#include "SdioDriver.h"

/**
    This function issues tuning command to card

    @param  SdioDevInterface 
    @param  Port
    @param  BufferAddress 

    @retval EFI_STATUS

**/
EFI_STATUS 
SdMmcTunning (
    IN  SDIO_DEVICE_INTERFACE   *SdioDevInterface,
    IN  UINT8                   Slot
)
{
    return EFI_SUCCESS;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
