//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2015, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//*****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*****************************************************************************
/** @file CrbSmiFlashHook.c

**/
//*****************************************************************************


#include <AmiDxeLib.h>
#include <AmiSmm.h>
#include <Protocol/SmiFlash.h>


#define CRB_FLASH_DEVICE_BASE_ADDRESS   (0xFFFFFFFF-FLASH_SIZE+1)
#define CRB_PRE_BB_BASE                 (FV_PRE_BB_BASE-CRB_FLASH_DEVICE_BASE_ADDRESS)
#define CRB_PRE_BB_END                  (CRB_PRE_BB_BASE+FV_PRE_BB_SIZE)
#if VERIFY_BOOT_SUPPORT
#undef  CRB_PRE_BB_BASE
#undef  CRB_PRE_BB_END
#define CRB_PRE_BB_BASE                 ((FV_PRE_BB_BASE-0x1000)-CRB_FLASH_DEVICE_BASE_ADDRESS)
#define CRB_PRE_BB_END                  (CRB_PRE_BB_BASE+FV_PRE_BB_SIZE)
#endif
#define CRB_MICROCODE_BASE              (MICROCODE_ADDRESS-CRB_FLASH_DEVICE_BASE_ADDRESS)
#define CRB_MICROCODE_END               (CRB_MICROCODE_BASE+MICROCODE_SIZE)


/**
    This function is SW SMI hook that sets Flash Block Description
    type for AMI AFU utility. 

    @param 
        SwSmiNum    - SW SMI value number
        Buffer      - Flash descriptor address

    @retval VOID

**/

VOID CrbUpdateBlockTypeId (
    IN UINT8    SwSmiNum,
    IN UINT64   Buffer )
{
    BLOCK_DESC  *BlockDesc;
    UINTN       i;

    
    // return if SW SMI value is not "Get Flash Info"
    if (SwSmiNum != SMIFLASH_GET_FLASH_INFO)
        return;

    BlockDesc = (BLOCK_DESC*)&((INFO_BLOCK*)Buffer)->Blocks;

    for (i = 0; i < ((INFO_BLOCK*)Buffer)->TotalBlocks; i++)
    {
        if ((BlockDesc[i].StartAddress >= CRB_PRE_BB_BASE) && (BlockDesc[i].StartAddress < CRB_PRE_BB_END))
            BlockDesc[i].Type = BOOT_BLOCK;;

        if ((BlockDesc[i].StartAddress >= CRB_MICROCODE_BASE) && (BlockDesc[i].StartAddress < CRB_MICROCODE_END))
            BlockDesc[i].Type = MAIN_BLOCK;;
    }
}


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2015, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
